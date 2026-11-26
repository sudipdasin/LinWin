// SPDX-License-Identifier: GPL-3.0-or-later
#include "clipboard_history_widget.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCoreApplication>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPointer>

#include <iostream>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("LinWinTest"));
    QCoreApplication::setApplicationName(QStringLiteral("ClipboardCenterTest"));
    QSettings settings;
    settings.clear();

    ClipboardHistoryWidget widget;
    widget.show();
    application.processEvents();
    const QStringList required{
        QStringLiteral("captureClipboard"), QStringLiteral("middleClickPaste"),
        QStringLiteral("clipboardSearch"), QStringLiteral("clipboardEntries"),
        QStringLiteral("pinClipboardItem"), QStringLiteral("clipboardQuickPick")
    };
    for (const QString &name : required) {
        if (widget.findChild<QWidget *>(name) == nullptr) {
            std::cerr << "Missing clipboard control: " << name.toStdString() << '\n';
            return 1;
        }
    }
    const auto *capture = widget.findChild<QCheckBox *>(QStringLiteral("captureClipboard"));
    const auto *middleClick = widget.findChild<QCheckBox *>(QStringLiteral("middleClickPaste"));
    if (capture == nullptr || capture->isChecked() || middleClick == nullptr || middleClick->isChecked()) {
        std::cerr << "Clipboard capture and middle-click paste must start off\n";
        return 1;
    }
    const auto *history = widget.findChild<QListWidget *>(QStringLiteral("clipboardEntries"));
    if (history == nullptr || history->count() != 0) {
        std::cerr << "Clipboard history should start empty\n";
        return 1;
    }
    auto *captureToggle = widget.findChild<QCheckBox *>(QStringLiteral("captureClipboard"));
    captureToggle->setChecked(true);
    QApplication::clipboard()->setText(QStringLiteral("first clip"));
    application.processEvents();
    const int savedCount = history->count();
    if (savedCount != 1) {
        std::cerr << "Enabled clipboard history should capture a new text clip\n";
        return 1;
    }
    widget.showQuickPicker(&widget);
    application.processEvents();
    QPointer<QDialog> picker = widget.findChild<QDialog *>(QStringLiteral("clipboardQuickPicker"));
    auto *quickSearch = widget.findChild<QLineEdit *>(QStringLiteral("clipboardQuickSearch"));
    auto *quickEntries = widget.findChild<QListWidget *>(QStringLiteral("clipboardQuickEntries"));
    if (picker == nullptr || !picker->isVisible() || quickSearch == nullptr
        || quickEntries == nullptr || quickEntries->count() != 1) {
        std::cerr << "Super+V quick picker should expose the saved clipboard item\n";
        return 1;
    }
    quickSearch->setText(QStringLiteral("missing"));
    if (!quickEntries->item(0)->isHidden()) {
        std::cerr << "Quick picker search should filter clipboard items\n";
        return 1;
    }
    quickSearch->setText(QStringLiteral("first"));
    if (quickEntries->item(0)->isHidden()) {
        std::cerr << "Quick picker search should restore matching clipboard items\n";
        return 1;
    }
    if (!QMetaObject::invokeMethod(&widget, "copyQuickPickEntry", Qt::DirectConnection,
                                   Q_ARG(QListWidgetItem *, quickEntries->item(0)))) {
        std::cerr << "Quick picker should activate a selected clipboard item\n";
        return 1;
    }
    application.processEvents();
    if (QApplication::clipboard()->text(QClipboard::Clipboard) != QStringLiteral("first clip")
        || (picker != nullptr && picker->isVisible())) {
        std::cerr << "Choosing a quick-pick item should copy it and close the popup\n";
        return 1;
    }

    auto *search = widget.findChild<QLineEdit *>(QStringLiteral("clipboardSearch"));
    if (search == nullptr) {
        std::cerr << "Clipboard search field was not created\n";
        return 1;
    }
    const auto sendMiddleClick = [search](const QPoint &position) {
        const QPoint globalPosition = search->mapToGlobal(position);
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(position), QPointF(globalPosition),
                          Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
        QApplication::sendEvent(search, &event);
    };
    search->setText(QStringLiteral("ABCD"));
    QPoint clickPosition(-1, search->height() / 2);
    for (int x = 0; x < search->width(); ++x) {
        if (search->cursorPositionAt(QPoint(x, clickPosition.y())) == 2) {
            clickPosition.setX(x);
            break;
        }
    }
    if (clickPosition.x() < 0) {
        std::cerr << "Could not find a test click point at the middle of the search text\n";
        return 1;
    }
    QApplication::clipboard()->setText(QStringLiteral("regular clipboard"), QClipboard::Clipboard);
    if (QApplication::clipboard()->supportsSelection()) {
        QApplication::clipboard()->setText(QStringLiteral("separate selection"), QClipboard::Selection);
    }
    sendMiddleClick(clickPosition);
    if (search->text() != QStringLiteral("ABCD")) {
        std::cerr << "Middle-click should be ignored by default in LinWin text fields\n";
        return 1;
    }
    middleClick->setChecked(true);
    sendMiddleClick(clickPosition);
    if (search->text() != QStringLiteral("ABregular clipboardCD")) {
        std::cerr << "Enabled middle-click should paste the regular clipboard, not PRIMARY selection\n";
        return 1;
    }
    middleClick->setChecked(false);

    widget.setComponentEnabled(false);
    if (widget.isEnabled()) {
        std::cerr << "Disabled clipboard component should not remain interactive\n";
        return 1;
    }
    QApplication::clipboard()->setText(QStringLiteral("must not be captured while disabled"));
    QMetaObject::invokeMethod(&widget, "captureClipboardText", Qt::DirectConnection);
    if (history->count() != savedCount) {
        std::cerr << "Disabled clipboard component must pause capture\n";
        return 1;
    }
    widget.setComponentEnabled(true);
    if (!widget.isEnabled()) {
        std::cerr << "Re-enabled clipboard component should become available\n";
        return 1;
    }
    settings.clear();
    return 0;
}
