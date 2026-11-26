// SPDX-License-Identifier: GPL-3.0-or-later
#include "settings_hub_widget.h"

#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QSettings>
#include <QSlider>
#include <QPushButton>
#include <QRegularExpression>
#include <QVector>

#include <iostream>

namespace {
int visibleCards(const SettingsHubWidget &widget)
{
    int count = 0;
    const auto cards = widget.findChildren<QFrame *>(QRegularExpression(QStringLiteral("^settingsCard_")));
    for (const QFrame *card : cards)
        if (!card->isHidden()) ++count;
    return count;
}

bool expect(bool condition, const char *message)
{
    if (condition) return true;
    std::cerr << message << '\n';
    return false;
}
}

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("LinWin tests"));
    QCoreApplication::setApplicationName(QStringLiteral("settings hub"));
    QSettings settings;
    settings.clear();
    SettingsHubWidget widget;
    widget.resize(900, 700);
    widget.show();
    application.processEvents();

    bool passed = true;
    const auto cards = widget.findChildren<QFrame *>(QRegularExpression(QStringLiteral("^settingsCard_")));
    passed &= expect(cards.size() == 11, "expected all 11 settings categories");
    passed &= expect(visibleCards(widget) == 11, "all categories should be visible before searching");

    auto *filesToggle = widget.findChild<QCheckBox *>(QStringLiteral("componentToggle_files"));
    auto *tasksToggle = widget.findChild<QCheckBox *>(QStringLiteral("componentToggle_task-manager"));
    auto *textScale = widget.findChild<QSlider *>(QStringLiteral("textScaleSlider"));
    auto *textScaleValue = widget.findChild<QLabel *>(QStringLiteral("textScaleValue"));
    auto *textScaleReset = widget.findChild<QPushButton *>(QStringLiteral("textScaleReset"));
    auto *automaticUpdates = widget.findChild<QCheckBox *>(QStringLiteral("automaticUpdatesCheck"));
    auto *checkUpdates = widget.findChild<QPushButton *>(QStringLiteral("checkUpdatesButton"));
    passed &= expect(filesToggle != nullptr && filesToggle->isChecked(),
                     "file manager should be enabled by default");
    passed &= expect(tasksToggle != nullptr && tasksToggle->isChecked(),
                     "Task Manager should be enabled by default");
    passed &= expect(automaticUpdates != nullptr && automaticUpdates->isChecked()
                         && checkUpdates != nullptr && checkUpdates->isEnabled(),
                     "Settings should offer a manual update check and daily automatic checks by default");
    if (automaticUpdates != nullptr) {
        automaticUpdates->setChecked(false);
        passed &= expect(!settings.value(QStringLiteral("updates/automaticCheck"), true).toBool(),
                         "Automatic update checks should be saved per account");
    }
    if (filesToggle != nullptr) filesToggle->setChecked(false);
    passed &= expect(!settings.value(QStringLiteral("components/files"), true).toBool(),
                     "component changes should persist to account settings");
    SettingsHubWidget reopened;
    auto *reopenedFilesToggle = reopened.findChild<QCheckBox *>(QStringLiteral("componentToggle_files"));
    passed &= expect(reopenedFilesToggle != nullptr && !reopenedFilesToggle->isChecked(),
                     "saved component preferences should load when settings reopen");

    passed &= expect(textScale != nullptr && textScaleValue != nullptr && textScaleReset != nullptr,
                     "settings should expose text-size controls");
    if (textScale != nullptr && textScaleValue != nullptr && textScaleReset != nullptr) {
        passed &= expect(textScale->minimum() == 85 && textScale->maximum() == 150,
                         "text scaling should stay within supported bounds");
        textScale->setValue(125);
        passed &= expect(textScaleValue->text() == QStringLiteral("125%")
                         && settings.value(QStringLiteral("appearance/textScalePercent")).toInt() == 125,
                         "text size should update its label and persist per account");
        passed &= expect(widget.styleSheet().contains(QStringLiteral("font-size: 20px")),
                         "text scaling should update LinWin settings typography");
        textScaleReset->click();
        passed &= expect(textScale->value() == 100
                         && settings.value(QStringLiteral("appearance/textScalePercent")).toInt() == 100,
                         "Reset should restore the standard text size");
        passed &= expect(widget.styleSheet().contains(QStringLiteral("font-size: 16px")),
                         "reset should restore default LinWin typography");
    }
    settings.clear();

    auto *search = widget.findChild<QLineEdit *>(QStringLiteral("settingsSearch"));
    passed &= expect(search != nullptr, "search field should have a stable object name");
    if (search != nullptr) {
        search->setText(QStringLiteral("bluetooth"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "Bluetooth search should narrow to one category");

        search->setText(QStringLiteral("wifi"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "Wi-Fi synonym should find Network settings");

        search->setText(QStringLiteral("no matching setting"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 0, "unmatched search should hide all cards");
        auto *empty = widget.findChild<QLabel *>(QStringLiteral("settingsEmptyState"));
        passed &= expect(empty != nullptr && !empty->isHidden(), "unmatched search should show helpful empty state");

        search->clear();
        application.processEvents();
        passed &= expect(visibleCards(widget) == 11, "clearing search should restore all categories");
    }
    return passed ? 0 : 1;
}
