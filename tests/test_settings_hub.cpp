// SPDX-License-Identifier: GPL-3.0-or-later
#include "settings_hub_widget.h"

#include <QApplication>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
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
    SettingsHubWidget widget;
    widget.resize(900, 700);
    widget.show();
    application.processEvents();

    bool passed = true;
    const auto cards = widget.findChildren<QFrame *>(QRegularExpression(QStringLiteral("^settingsCard_")));
    passed &= expect(cards.size() == 11, "expected all 11 settings categories");
    passed &= expect(visibleCards(widget) == 11, "all categories should be visible before searching");

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
