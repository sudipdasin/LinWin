// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QDebug>
#include <QLabel>
#include <QFrame>

namespace {
bool require(bool condition, const char *message)
{
    if (condition) return true;
    qCritical() << message;
    return false;
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("LinWinTests"));
    QCoreApplication::setApplicationName(QStringLiteral("ResponsiveTaskbar"));
    QSettings settings;
    settings.setValue(QStringLiteral("components/start-menu"), true);
    settings.setValue(QStringLiteral("components/files"), true);
    settings.setValue(QStringLiteral("components/task-manager"), true);
    settings.setValue(QStringLiteral("components/clipboard"), true);

    MainWindow window(true);
    window.show();
    window.resize(1180, 760);
    app.processEvents();

    auto *more = window.findChild<QPushButton *>(QStringLiteral("taskbarOverflowButton"));
    auto *clipboard = window.findChild<QPushButton *>(QStringLiteral("clipboardButton"));
    QPushButton *settingsButton = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
        if (button->text() == QStringLiteral("Settings")) settingsButton = button;
    }
    auto *signOut = window.findChild<QPushButton *>(QStringLiteral("signOutButton"));
    auto *switcher = window.findChild<QScrollArea *>(QStringLiteral("windowSwitcher"));
    auto *systemStatus = window.findChild<QLabel *>(QStringLiteral("panelStatus"));
    auto *taskbar = window.findChild<QFrame *>(QStringLiteral("taskbar"));
    auto *clock = window.findChild<QLabel *>(QStringLiteral("panelClock"));
    auto *pages = window.findChild<QStackedWidget *>(QStringLiteral("shellPages"));

    if (!require(more && clipboard && settingsButton && signOut && switcher && systemStatus && pages
                     && taskbar && clock,
                 "Expected taskbar controls were not created.")) return 1;
    if (!require(more->isHidden() && clipboard->isVisible() && settingsButton->isVisible()
                     && signOut->isVisible() && switcher->isVisible() && systemStatus->isVisible(),
                 "Wide taskbar should show its full navigation and window details.")) return 1;

    window.resize(480, 600);
    app.processEvents();
    if (!require(more->isVisible() && clipboard->isHidden() && settingsButton->isHidden()
                     && signOut->isHidden() && switcher->isHidden() && systemStatus->isHidden(),
                 "Narrow taskbar should move secondary controls into its overflow menu.")) return 1;
    for (auto *widget : {static_cast<QWidget *>(more), static_cast<QWidget *>(clock)}) {
        if (!require(widget->geometry().left() >= 0 && widget->geometry().right() < taskbar->width(),
                     "Visible compact taskbar controls must remain inside the panel.")) return 1;
    }

    const auto actions = more->menu()->actions();
    if (!require(actions.size() == 3 && actions.at(0)->isVisible() && actions.at(1)->isVisible()
                     && actions.at(2)->isVisible(),
                 "Narrow taskbar overflow should expose Clipboard, Settings, and Sign out.")) return 1;
    actions.at(0)->trigger();
    app.processEvents();
    if (!require(pages->currentIndex() == 3, "Overflow Clipboard action should open Clipboard.")) return 1;
    actions.at(1)->trigger();
    app.processEvents();
    if (!require(pages->currentIndex() == 4, "Overflow Settings action should open Settings.")) return 1;

    return 0;
}
