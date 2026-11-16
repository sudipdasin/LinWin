// SPDX-License-Identifier: GPL-3.0-or-later
#include "clipboard_history_widget.h"

#include <QCheckBox>
#include <QClipboard>
#include <QAbstractItemView>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QVBoxLayout>

namespace {
constexpr qsizetype kMaximumEntries = 50;
constexpr qsizetype kMaximumTextLength = 20000;
const QString kHistorySetting = QStringLiteral("clipboard/history");
const QString kCaptureSetting = QStringLiteral("clipboard/historyEnabled");

QString previewText(QString text)
{
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    text = text.trimmed();
    constexpr qsizetype kPreviewLength = 180;
    if (text.size() > kPreviewLength) {
        text = text.left(kPreviewLength - 1) + QChar(0x2026);
    }
    return text;
}
}

ClipboardHistoryWidget::ClipboardHistoryWidget(QWidget *parent)
    : QWidget(parent), clipboard_(QApplication::clipboard())
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(14);

    auto *title = new QLabel(tr("Clipboard history"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *description = new QLabel(
        tr("Keep recent text clips handy and copy them back with one click."), this);
    description->setObjectName(QStringLiteral("intro"));
    description->setWordWrap(true);

    auto *privacyNote = new QLabel(
        tr("History is off by default. Clipboard text can include passwords or private information. "
           "When enabled, LinWin saves up to 50 recent text items in this user's local application settings. "
           "Capture works only while LinWin is open and depends on desktop-session permissions."), this);
    privacyNote->setObjectName(QStringLiteral("privacyNote"));
    privacyNote->setWordWrap(true);

    captureCheckBox_ = new QCheckBox(tr("Enable clipboard history"), this);
    captureCheckBox_->setObjectName(QStringLiteral("captureClipboard"));

    searchField_ = new QLineEdit(this);
    searchField_->setObjectName(QStringLiteral("clipboardSearch"));
    searchField_->setPlaceholderText(tr("Search clipboard history"));
    searchField_->setClearButtonEnabled(true);

    historyList_ = new QListWidget(this);
    historyList_->setObjectName(QStringLiteral("clipboardEntries"));
    historyList_->setSelectionMode(QAbstractItemView::SingleSelection);
    historyList_->setUniformItemSizes(true);

    auto *actions = new QHBoxLayout;
    auto *copyButton = new QPushButton(tr("Copy selected"), this);
    auto *removeButton = new QPushButton(tr("Remove"), this);
    auto *clearButton = new QPushButton(tr("Clear history"), this);
    copyButton->setObjectName(QStringLiteral("primaryAction"));
    actions->addWidget(copyButton);
    actions->addWidget(removeButton);
    actions->addStretch();
    actions->addWidget(clearButton);

    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName(QStringLiteral("clipboardStatus"));
    statusLabel_->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addWidget(privacyNote);
    layout->addWidget(captureCheckBox_);
    layout->addWidget(searchField_);
    layout->addWidget(historyList_, 1);
    layout->addLayout(actions);
    layout->addWidget(statusLabel_);

    QSettings settings;
    history_ = settings.value(kHistorySetting).toStringList();
    while (history_.size() > kMaximumEntries) {
        history_.removeLast();
    }
    captureCheckBox_->setChecked(settings.value(kCaptureSetting, false).toBool());
    refreshList();

    connect(clipboard_, &QClipboard::dataChanged,
            this, &ClipboardHistoryWidget::captureClipboardText);
    connect(captureCheckBox_, &QCheckBox::toggled,
            this, &ClipboardHistoryWidget::updateCaptureState);
    connect(searchField_, &QLineEdit::textChanged,
            this, &ClipboardHistoryWidget::filterEntries);
    connect(copyButton, &QPushButton::clicked,
            this, &ClipboardHistoryWidget::copySelectedEntry);
    connect(removeButton, &QPushButton::clicked,
            this, &ClipboardHistoryWidget::removeSelectedEntry);
    connect(clearButton, &QPushButton::clicked,
            this, &ClipboardHistoryWidget::clearHistory);
    connect(historyList_, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem *) { copySelectedEntry(); });

    updateCaptureState(captureCheckBox_->isChecked());
}

void ClipboardHistoryWidget::captureClipboardText()
{
    if (!captureCheckBox_->isChecked() || clipboard_ == nullptr) {
        return;
    }

    const QString text = clipboard_->text(QClipboard::Clipboard);
    if (text.isEmpty() || text.size() > kMaximumTextLength) {
        if (text.size() > kMaximumTextLength) {
            statusLabel_->setText(tr("This text clip is too large to save; the history limit is 20,000 characters per item."));
        }
        return;
    }

    if (!history_.isEmpty() && history_.constFirst() == text) {
        return;
    }

    history_.removeAll(text);
    history_.prepend(text);
    while (history_.size() > kMaximumEntries) {
        history_.removeLast();
    }
    saveHistory();
    refreshList();
    statusLabel_->setText(tr("Saved a text clip. Double-click an item or select it and choose Copy selected."));
}

void ClipboardHistoryWidget::copySelectedEntry()
{
    const auto *item = historyList_->currentItem();
    if (item == nullptr || clipboard_ == nullptr) {
        return;
    }
    clipboard_->setText(item->data(Qt::UserRole).toString(), QClipboard::Clipboard);
    statusLabel_->setText(tr("Copied the selected item to the clipboard."));
}

void ClipboardHistoryWidget::removeSelectedEntry()
{
    const auto *item = historyList_->currentItem();
    if (item == nullptr) {
        statusLabel_->setText(tr("Select an item to remove."));
        return;
    }
    history_.removeAll(item->data(Qt::UserRole).toString());
    saveHistory();
    refreshList();
    statusLabel_->setText(tr("Removed the selected item."));
}

void ClipboardHistoryWidget::clearHistory()
{
    if (history_.isEmpty()) {
        statusLabel_->setText(tr("Clipboard history is already empty."));
        return;
    }

    const auto answer = QMessageBox::question(
        this, tr("Clear clipboard history?"),
        tr("This permanently removes all saved clipboard text from LinWin's local history."),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (answer != QMessageBox::Yes) {
        return;
    }

    history_.clear();
    saveHistory();
    refreshList();
    statusLabel_->setText(tr("Clipboard history cleared."));
}

void ClipboardHistoryWidget::filterEntries(const QString &query)
{
    const QString needle = query.trimmed();
    for (int row = 0; row < historyList_->count(); ++row) {
        auto *item = historyList_->item(row);
        item->setHidden(!item->data(Qt::UserRole).toString().contains(needle, Qt::CaseInsensitive));
    }
}

void ClipboardHistoryWidget::saveHistory() const
{
    QSettings settings;
    settings.setValue(kHistorySetting, history_);
}

void ClipboardHistoryWidget::refreshList()
{
    historyList_->clear();
    for (const QString &text : history_) {
        auto *item = new QListWidgetItem(previewText(text), historyList_);
        item->setData(Qt::UserRole, text);
        item->setToolTip(text);
    }
    filterEntries(searchField_->text());
    if (history_.isEmpty()) {
        statusLabel_->setText(tr("No saved clips yet. Enable history, then copy text while LinWin is open."));
    }
}

void ClipboardHistoryWidget::updateCaptureState(bool enabled)
{
    QSettings settings;
    settings.setValue(kCaptureSetting, enabled);
    statusLabel_->setText(enabled
        ? tr("Clipboard monitoring is on while LinWin is open. Stored history is not encrypted.")
        : tr("Clipboard monitoring is off. Existing saved items remain until you clear them."));
}
