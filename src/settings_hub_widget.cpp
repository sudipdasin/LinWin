// SPDX-License-Identifier: GPL-3.0-or-later
#include "settings_hub_widget.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStandardPaths>
#include <QStyle>

#include <algorithm>
#include <QVBoxLayout>

namespace {
struct Candidate {
    QString program;
    QStringList arguments;
};

struct Category {
    QString title;
    QString description;
    QString keywords;
    QVector<Candidate> candidates;
};

QVector<Category> categories()
{
    return {
        {QStringLiteral("Display"), QStringLiteral("Screen resolution, scaling, orientation, and multiple monitors."),
         QStringLiteral("screen resolution scaling monitor display brightness"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("display")}},
             {QStringLiteral("xfce4-display-settings"), {}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_kscreen")}},
             {QStringLiteral("mate-display-properties"), {}}
         }},
        {QStringLiteral("Network & Wi-Fi"), QStringLiteral("Wi-Fi, wired connections, VPN, and network options."),
         QStringLiteral("network wifi wireless ethernet vpn internet connections"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("network")}},
             {QStringLiteral("nm-connection-editor"), {}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_networkmanagement")}},
             {QStringLiteral("cinnamon-settings"), {QStringLiteral("network")}}
         }},
        {QStringLiteral("Bluetooth"), QStringLiteral("Pair and manage devices using the desktop's Bluetooth tools."),
         QStringLiteral("bluetooth pair headphones mouse keyboard devices"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("bluetooth")}},
             {QStringLiteral("blueman-manager"), {}},
             {QStringLiteral("kcmshell6"), {QStringLiteral("kcm_bluetooth")}},
             {QStringLiteral("kcmshell5"), {QStringLiteral("kcm_bluetooth")}}
         }},
        {QStringLiteral("Sound"), QStringLiteral("Choose speakers, headphones, microphones, and volume."),
         QStringLiteral("audio sound volume speaker microphone input output"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("sound")}},
             {QStringLiteral("pavucontrol"), {}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_pulseaudio")}},
             {QStringLiteral("mate-volume-control"), {}}
         }},
        {QStringLiteral("Printers"), QStringLiteral("Add printers and review print queues and options."),
         QStringLiteral("printer printing queue scanner"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("printers")}},
             {QStringLiteral("system-config-printer"), {}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_printer_manager")}}
         }},
        {QStringLiteral("Power"), QStringLiteral("Battery, power-saving, sleep, and screen timeout settings."),
         QStringLiteral("battery power sleep suspend energy timeout"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("power")}},
             {QStringLiteral("xfce4-power-manager-settings"), {}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_powerdevil")}},
             {QStringLiteral("mate-power-preferences"), {}}
         }},
        {QStringLiteral("Accessibility"), QStringLiteral("Text size, assistive technology, keyboard, and contrast options."),
         QStringLiteral("accessibility magnifier screen reader contrast keyboard text size"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("universal-access")}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_access")}},
             {QStringLiteral("mate-keyboard-properties"), {}}
         }},
        {QStringLiteral("Appearance"), QStringLiteral("Themes, colors, fonts, icons, and desktop appearance."),
         QStringLiteral("appearance theme colors fonts icons wallpaper"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("appearance")}},
             {QStringLiteral("xfce4-appearance-settings"), {}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_lookandfeel")}},
             {QStringLiteral("mate-appearance-properties"), {}}
         }},
        {QStringLiteral("Users & Accounts"), QStringLiteral("Open your desktop's account and user management panel."),
         QStringLiteral("users accounts password login account"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("user-accounts")}},
             {QStringLiteral("cinnamon-settings"), {QStringLiteral("users")}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_users")}}
         }},
        {QStringLiteral("Updates & Software"), QStringLiteral("Open the update manager or software center provided by this desktop."),
         QStringLiteral("updates upgrade software install applications packages"), {
             {QStringLiteral("mintupdate"), {}},
             {QStringLiteral("gnome-software"), {}},
             {QStringLiteral("plasma-discover"), {}},
             {QStringLiteral("pamac-manager"), {}}
         }},
        {QStringLiteral("Date & Time"), QStringLiteral("Time zone, clock, date, and time-format settings."),
         QStringLiteral("date time clock timezone calendar"), {
             {QStringLiteral("gnome-control-center"), {QStringLiteral("datetime")}},
             {QStringLiteral("cinnamon-settings"), {QStringLiteral("datetime")}},
             {QStringLiteral("systemsettings"), {QStringLiteral("kcm_clock")}},
             {QStringLiteral("time-admin"), {}}
         }}
    };
}

QString findAvailableProgram(const QVector<Candidate> &candidates, QStringList *arguments)
{
    QVector<Candidate> ordered = candidates;
    const QString desktop = (qEnvironmentVariable("XDG_CURRENT_DESKTOP") + QLatin1Char(' ')
        + qEnvironmentVariable("DESKTOP_SESSION")).toCaseFolded();
    const auto score = [&desktop](const Candidate &candidate) {
        const QString &program = candidate.program;
        if ((desktop.contains(QStringLiteral("kde")) || desktop.contains(QStringLiteral("plasma")))
            && (program == QStringLiteral("systemsettings") || program.startsWith(QStringLiteral("kcmshell")))) return 0;
        if (desktop.contains(QStringLiteral("xfce")) && program.startsWith(QStringLiteral("xfce4-"))) return 0;
        if (desktop.contains(QStringLiteral("mate")) && program.startsWith(QStringLiteral("mate-"))) return 0;
        if (desktop.contains(QStringLiteral("cinnamon")) && program == QStringLiteral("cinnamon-settings")) return 0;
        if (desktop.contains(QStringLiteral("gnome")) && program == QStringLiteral("gnome-control-center")) return 0;
        return 1;
    };
    std::stable_sort(ordered.begin(), ordered.end(), [&score](const Candidate &left, const Candidate &right) {
        return score(left) < score(right);
    });
    for (const Candidate &candidate : ordered) {
        const QString path = QStandardPaths::findExecutable(candidate.program);
        if (!path.isEmpty()) {
            *arguments = candidate.arguments;
            return path;
        }
    }
    return {};
}
}

SettingsHubWidget::SettingsHubWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("settingsHub"));
    setStyleSheet(QStringLiteral(
        "#settingsSearch { background: white; border: 1px solid #cfd9e8; border-radius: 8px; padding: 10px 12px; }"
        "#settingsStatus { color: #64738a; font-size: 12px; }"
        "QFrame[linwinSettingsCard=\"true\"] { background: white; border: 1px solid #e0e6ef; border-radius: 11px; }"
        "#settingsCardTitle { color: #172033; font-size: 16px; font-weight: 650; }"
        "#settingsCardDescription { color: #526078; font-size: 13px; }"
        "#settingsAvailability { color: #287345; font-size: 12px; }"
        "#settingsAvailability[available=\"false\"] { color: #7b8493; }"
        "#settingsOpenButton { background: #3568d4; color: white; border: 0; border-radius: 6px; padding: 7px 13px; font-weight: 600; }"
        "#settingsOpenButton:disabled { background: #e8edf4; color: #788397; }"
        "#settingsEmptyState { color: #64738a; background: white; border: 1px dashed #cfd9e8; border-radius: 10px; padding: 22px; }"));
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 24, 28, 20);
    outer->setSpacing(12);

    auto *eyebrow = new QLabel(tr("LINWIN SETTINGS"), this);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    auto *heading = new QLabel(tr("Settings"), this);
    heading->setObjectName(QStringLiteral("pageTitle"));
    auto *description = new QLabel(
        tr("Find a setting and open the control panel provided by your Linux desktop. LinWin does not change system settings behind your back."),
        this);
    description->setObjectName(QStringLiteral("intro"));
    description->setWordWrap(true);
    outer->addWidget(eyebrow);
    outer->addWidget(heading);
    outer->addWidget(description);

    search_ = new QLineEdit(this);
    search_->setObjectName(QStringLiteral("settingsSearch"));
    search_->setClearButtonEnabled(true);
    search_->setPlaceholderText(tr("Search display, Wi-Fi, Bluetooth, sound, power…"));
    search_->setAccessibleName(tr("Search settings"));
    outer->addWidget(search_);

    resultStatus_ = new QLabel(this);
    resultStatus_->setObjectName(QStringLiteral("settingsStatus"));
    resultStatus_->setAccessibleName(tr("Settings search results"));
    outer->addWidget(resultStatus_);

    auto *scroll = new QScrollArea(this);
    scroll->setObjectName(QStringLiteral("settingsScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("settingsCards"));
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    grid_ = new QGridLayout;
    grid_->setObjectName(QStringLiteral("settingsCardGrid"));
    grid_->setContentsMargins(0, 0, 0, 0);
    grid_->setSpacing(12);
    contentLayout->addLayout(grid_);

    emptyState_ = new QLabel(tr("No settings match your search. Try another word."), content);
    emptyState_->setObjectName(QStringLiteral("settingsEmptyState"));
    emptyState_->setWordWrap(true);
    emptyState_->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(emptyState_);
    contentLayout->addStretch();
    scroll->setWidget(content);
    outer->addWidget(scroll, 1);

    const QVector<Category> allCategories = categories();
    cards_.reserve(allCategories.size());
    targets_.reserve(allCategories.size());

    for (int index = 0; index < allCategories.size(); ++index) {
        const Category &category = allCategories.at(index);
        QStringList arguments;
        const QString program = findAvailableProgram(category.candidates, &arguments);
        targets_.append({program, arguments});

        auto *card = new QFrame(content);
        card->setObjectName(QStringLiteral("settingsCard_%1").arg(index));
        card->setProperty("categoryTitle", category.title);
        card->setProperty("linwinSettingsCard", true);
        card->setFrameShape(QFrame::StyledPanel);
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 14, 16, 14);
        cardLayout->setSpacing(8);

        auto *title = new QLabel(category.title, card);
        title->setObjectName(QStringLiteral("settingsCardTitle"));
        title->setWordWrap(true);
        auto *details = new QLabel(category.description, card);
        details->setObjectName(QStringLiteral("settingsCardDescription"));
        details->setWordWrap(true);
        details->setMinimumHeight(38);
        auto *footer = new QHBoxLayout;
        auto *availability = new QLabel(program.isEmpty() ? tr("No compatible tool found") : tr("Available on this system"), card);
        availability->setObjectName(QStringLiteral("settingsAvailability"));
        availability->setProperty("available", !program.isEmpty());
        availability->setWordWrap(true);
        auto *openButton = new QPushButton(program.isEmpty() ? tr("Unavailable") : tr("Open"), card);
        openButton->setObjectName(QStringLiteral("settingsOpenButton"));
        openButton->setEnabled(!program.isEmpty());
        openButton->setAccessibleName(program.isEmpty()
            ? tr("%1 settings are unavailable").arg(category.title)
            : tr("Open %1 settings").arg(category.title));
        footer->addWidget(availability, 1);
        footer->addWidget(openButton);
        cardLayout->addWidget(title);
        cardLayout->addWidget(details);
        cardLayout->addLayout(footer);

        CategoryCard entry;
        entry.frame = card;
        entry.searchText = (category.title + QLatin1Char(' ') + category.description + QLatin1Char(' ') + category.keywords).toCaseFolded();
        cards_.append(entry);
        connect(openButton, &QPushButton::clicked, this, [this, index]() { openCategory(index); });
    }

    connect(search_, &QLineEdit::textChanged, this, &SettingsHubWidget::filterCategories);
    filterCategories({});
}

void SettingsHubWidget::focusSearch()
{
    search_->setFocus(Qt::ShortcutFocusReason);
    search_->selectAll();
}

void SettingsHubWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    arrangeCards();
}

void SettingsHubWidget::filterCategories(const QString &query)
{
    const QString needle = query.trimmed().toCaseFolded();
    int visibleCount = 0;
    int availableCount = 0;
    for (int index = 0; index < cards_.size(); ++index) {
        const bool matches = needle.isEmpty() || cards_.at(index).searchText.contains(needle);
        cards_.at(index).frame->setVisible(matches);
        if (matches) {
            ++visibleCount;
            if (!targets_.at(index).program.isEmpty()) ++availableCount;
        }
    }
    emptyState_->setVisible(visibleCount == 0);
    resultStatus_->setText(tr("%1 settings shown · %2 available to open")
        .arg(visibleCount).arg(availableCount));
    arrangeCards();
}

void SettingsHubWidget::arrangeCards()
{
    if (grid_ == nullptr || cards_.isEmpty()) return;
    while (QLayoutItem *item = grid_->takeAt(0)) delete item;
    const int columns = width() >= 840 ? 2 : 1;
    int visibleIndex = 0;
    for (const CategoryCard &card : cards_) {
        if (card.frame->isHidden()) continue;
        const int row = visibleIndex / columns;
        const int column = visibleIndex % columns;
        grid_->addWidget(card.frame, row, column);
        ++visibleIndex;
    }
    for (int column = 0; column < columns; ++column)
        grid_->setColumnStretch(column, 1);
}

void SettingsHubWidget::openCategory(int index)
{
    if (index < 0 || index >= targets_.size()) return;
    const LaunchTarget &target = targets_.at(index);
    if (target.program.isEmpty()) return;
    if (!QProcess::startDetached(target.program, target.arguments)) {
        auto *availability = cards_.at(index).frame->findChild<QLabel *>(QStringLiteral("settingsAvailability"));
        if (availability != nullptr) {
            availability->setText(tr("Could not open this tool. It may have been removed."));
            availability->setProperty("available", false);
            availability->style()->unpolish(availability);
            availability->style()->polish(availability);
        }
    }
}
