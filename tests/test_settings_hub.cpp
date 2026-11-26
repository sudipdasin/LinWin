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
#include <QDialog>
#include <QTimer>
#include <QComboBox>
#include <QTableWidget>
#include <QGuiApplication>

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
    auto *themes = widget.findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
    passed &= expect(themes && themes->count() == 7 && themes->currentData().toString() == QStringLiteral("neon"), "Seven themes with Neon Fluent as default.");
    if (themes) {
        themes->setCurrentIndex(themes->findData(QStringLiteral("minimal")));
        passed &= expect(settings.value(QStringLiteral("appearance/theme")).toString() == QStringLiteral("minimal"), "Theme choice persists.");
        themes->setCurrentIndex(0);
    }
    const auto cards = widget.findChildren<QFrame *>(QRegularExpression(QStringLiteral("^settingsCard_")));
    passed &= expect(cards.size() == 12, "expected all 12 settings categories");
    passed &= expect(visibleCards(widget) == 12, "all categories should be visible before searching");

    auto *filesToggle = widget.findChild<QCheckBox *>(QStringLiteral("componentToggle_files"));
    auto *tasksToggle = widget.findChild<QCheckBox *>(QStringLiteral("componentToggle_task-manager"));
    auto *textScale = widget.findChild<QSlider *>(QStringLiteral("textScaleSlider"));
    auto *textScaleValue = widget.findChild<QLabel *>(QStringLiteral("textScaleValue"));
    auto *textScaleReset = widget.findChild<QPushButton *>(QStringLiteral("textScaleReset"));
    passed &= expect(filesToggle != nullptr && filesToggle->isChecked(),
                     "file manager should be enabled by default");
    passed &= expect(tasksToggle != nullptr && tasksToggle->isChecked(),
                     "Task Manager should be enabled by default");
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

    auto *info = widget.findChild<QPushButton *>(QStringLiteral("linwinDisplayInfoButton"));
    passed &= expect(info != nullptr, "Display information must be available without a host control panel.");
    if (info) {
        QTimer::singleShot(0, &widget, [&]() {
            auto *dialog = widget.findChild<QDialog *>(QStringLiteral("linwinDisplayInformation"));
            auto *table = dialog ? dialog->findChild<QTableWidget *>(QStringLiteral("linwinDisplayTable")) : nullptr;
            passed &= expect(table && table->rowCount() == QGuiApplication::screens().size(), "Display info must enumerate real Qt screens.");
            if (dialog) dialog->reject();
            else QApplication::closeAllWindows();
        });
        info->click();
    }

    auto *search = widget.findChild<QLineEdit *>(QStringLiteral("settingsSearch"));
    passed &= expect(search != nullptr, "search field should have a stable object name");
    if (search != nullptr) {
        search->setText(QStringLiteral("bluetooth"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "Bluetooth search should narrow to one category");

        search->setText(QStringLiteral("wifi"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "Wi-Fi synonym should find Network settings");

        search->setText(QStringLiteral("linwin updates"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "LinWin update search should find its own category.");
        bool foundUpdateCard = false;
        for (const QFrame *card : cards)
            if (!card->isHidden() && card->property("categoryTitle").toString() == QStringLiteral("LinWin Updates"))
                foundUpdateCard = true;
        passed &= expect(foundUpdateCard, "LinWin Updates must be distinct from host software updates.");
        for (QFrame *card : cards) {
            if (card->isHidden() || card->property("categoryTitle").toString() != QStringLiteral("LinWin Updates")) continue;
            auto *open = card->findChild<QPushButton *>(QStringLiteral("settingsOpenButton"));
            passed &= expect(open && open->isEnabled(), "Embedded update page must not depend on companion installation.");
            if (open && open->isEnabled()) {
                QTimer::singleShot(0, &widget, [&]() {
                    auto *dialog = widget.findChild<QDialog *>(QStringLiteral("linwinEmbeddedUpdateDialog"));
                    passed &= expect(dialog && dialog->findChild<QPushButton *>(QStringLiteral("linwinCheckForUpdates")), "Update opens a native LinWin dialog.");
                    if (dialog) dialog->reject();
                    else QApplication::closeAllWindows();
                });
                open->click();
            }
        }

        search->setText(QStringLiteral("no matching setting"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 0, "unmatched search should hide all cards");
        auto *empty = widget.findChild<QLabel *>(QStringLiteral("settingsEmptyState"));
        passed &= expect(empty != nullptr && !empty->isHidden(), "unmatched search should show helpful empty state");

        search->clear();
        application.processEvents();
        passed &= expect(visibleCards(widget) == 12, "clearing search should restore all categories");
    }
    return passed ? 0 : 1;
}
