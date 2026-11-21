// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "workspace_state.h"

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
    const auto workspaces = parseWorkspaceList(
        QStringLiteral("0 * DG: 1920x1080  VP: 0,0  WA: 0,0 1920x1040  N/A Main\n"
                       "1 - DG: 1920x1080  VP: 0,0  WA: 0,0 1920x1040  N/A Research\n"
                       "malformed desktop row\n"));
    if (!require(workspaces.size() == 2 && workspaces.at(0).index == 0
                     && workspaces.at(0).name == QStringLiteral("Main") && workspaces.at(0).current
                     && workspaces.at(1).index == 1
                     && workspaces.at(1).name == QStringLiteral("Research")
                     && !workspaces.at(1).current,
                 "Workspace parsing should preserve names, indexes, and the active desktop.")) return 1;
    const auto unnamedWorkspace = parseWorkspaceList(
        QStringLiteral("2 - DG: 1920x1080  VP: 0,0  WA: 0,0 1920x1040\n"));
    if (!require(unnamedWorkspace.size() == 1
                     && unnamedWorkspace.first().name == QStringLiteral("Desktop 3"),
                 "Unnamed workspaces should receive a friendly numbered label.")) return 1;

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
    auto *workspaceButton = window.findChild<QPushButton *>(QStringLiteral("workspaceButton"));
    QPushButton *clipboard = nullptr;
    QPushButton *settingsButton = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
        if (button->text() == QStringLiteral("Clipboard")) clipboard = button;
        if (button->text() == QStringLiteral("Settings")) settingsButton = button;
    }
    auto *signOut = window.findChild<QPushButton *>(QStringLiteral("signOutButton"));
    auto *switcher = window.findChild<QScrollArea *>(QStringLiteral("windowSwitcher"));
    auto *systemStatus = window.findChild<QLabel *>(QStringLiteral("panelStatus"));
    auto *taskbar = window.findChild<QFrame *>(QStringLiteral("taskbar"));
    auto *clock = window.findChild<QLabel *>(QStringLiteral("panelClock"));
    auto *pages = window.findChild<QStackedWidget *>(QStringLiteral("shellPages"));

    if (!require(more && workspaceButton && clipboard && settingsButton && signOut && switcher && systemStatus && pages
                     && taskbar && clock,
                 "Expected taskbar controls were not created.")) return 1;
    if (!require(more->isHidden() && workspaceButton->isVisible() && clipboard->isVisible() && settingsButton->isVisible()
                     && signOut->isVisible() && switcher->isVisible() && systemStatus->isVisible(),
                 "Wide taskbar should show its full navigation and window details.")) return 1;

    window.resize(480, 600);
    app.processEvents();
    if (!require(more->isVisible() && workspaceButton->isHidden() && clipboard->isHidden() && settingsButton->isHidden()
                     && signOut->isHidden() && switcher->isHidden() && systemStatus->isHidden(),
                 "Narrow taskbar should move secondary controls into its overflow menu.")) return 1;
    for (auto *widget : {static_cast<QWidget *>(more), static_cast<QWidget *>(clock)}) {
        if (!require(widget->geometry().left() >= 0 && widget->geometry().right() < taskbar->width(),
                     "Visible compact taskbar controls must remain inside the panel.")) return 1;
    }

    const auto actions = more->menu()->actions();
    if (!require(actions.size() == 4 && actions.at(0)->isVisible() && actions.at(1)->isVisible()
                     && actions.at(2)->isVisible()
                     && actions.at(3)->text() == QStringLiteral("Virtual desktops…"),
                 "Narrow taskbar overflow should expose core actions and a virtual desktop entry.")) return 1;
    actions.at(0)->trigger();
    app.processEvents();
    if (!require(pages->currentIndex() == 3, "Overflow Clipboard action should open Clipboard.")) return 1;
    actions.at(1)->trigger();
    app.processEvents();
    if (!require(pages->currentIndex() == 4, "Overflow Settings action should open Settings.")) return 1;

    return 0;
}
