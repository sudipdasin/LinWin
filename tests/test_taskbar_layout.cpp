// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "desktop_surface.h"
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
#include <QListWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QDir>
#include <QPixmap>
#include <QDialog>

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
    settings.clear();
    settings.setValue(QStringLiteral("appearance/theme"), QStringLiteral("neon"));
    settings.setValue(QStringLiteral("appearance/independentWindows"), false);
    settings.setValue(QStringLiteral("components/start-menu"), true);
    settings.setValue(QStringLiteral("components/files"), true);
    settings.setValue(QStringLiteral("components/task-manager"), true);
    settings.setValue(QStringLiteral("components/clipboard"), true);

    MainWindow window(true, false);
    window.show();
    window.resize(1180, 760);
    app.processEvents();

    auto *more = window.findChild<QPushButton *>(QStringLiteral("taskbarOverflowButton"));
    auto *workspaceButton = window.findChild<QPushButton *>(QStringLiteral("workspaceButton"));
    auto *desktopButton = window.findChild<QPushButton *>(QStringLiteral("desktopButton"));
    QPushButton *clipboard = nullptr;
    QPushButton *settingsButton = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
        if (button->text() == QStringLiteral("Clipboard")) clipboard = button;
        if (button->text() == QStringLiteral("Settings")) settingsButton = button;
    }
    auto *returnDesktop = window.findChild<QPushButton *>(QStringLiteral("returnDesktopButton"));
    auto *closeLinWin = window.findChild<QPushButton *>(QStringLiteral("closeLinWinButton"));
    auto *switcher = window.findChild<QScrollArea *>(QStringLiteral("windowSwitcher"));
    auto *systemStatus = window.findChild<QLabel *>(QStringLiteral("panelStatus"));
    auto *taskbar = window.findChild<QFrame *>(QStringLiteral("taskbar"));
    auto *clock = window.findChild<QLabel *>(QStringLiteral("panelClock"));
    auto *pages = window.findChild<QStackedWidget *>(QStringLiteral("shellPages"));

    auto *desktopCanvas = window.findChild<QWidget *>(QStringLiteral("windowsDesktop"));
    auto *surface = static_cast<DesktopSurface *>(desktopCanvas);
    if (!require(surface != nullptr, "Desktop visual surface should exist.")) return 1;
    surface->setEffectsEnabled(false);
    surface->setGpuPreferred(false);
    if (!require(!surface->effectsEnabled() && !surface->gpuPreferred(), "Desktop effects and OpenGL should be independently disableable.")) return 1;
    if (!require(more && workspaceButton && desktopButton && clipboard && settingsButton && returnDesktop && closeLinWin
                     && switcher && systemStatus && pages && desktopCanvas
                     && taskbar && clock,
                 "Expected taskbar controls were not created.")) return 1;
    if (!require(window.windowFlags().testFlag(Qt::FramelessWindowHint),
                 "The desktop overlay should hide the native window frame.")) return 1;
    if (!require(more->isHidden() && workspaceButton->isVisible() && desktopButton->isVisible()
                     && clipboard->isVisible() && settingsButton->isVisible()
                     && returnDesktop->isVisible() && closeLinWin->isVisible()
                     && switcher->isVisible() && systemStatus->isVisible(),
                 "Wide taskbar should show its full navigation and window details.")) return 1;

    auto *desktopIcons = window.findChild<QListWidget *>(QStringLiteral("desktopIcons"));
    if (!require(desktopIcons && desktopIcons->count() >= 3
                     && desktopIcons->viewMode() == QListView::IconMode
                     && desktopIcons->selectionMode() == QAbstractItemView::ExtendedSelection,
                 "Desktop should present selectable workspace icons.")) return 1;
    desktopIcons->setCurrentRow(0);
    if (!require(pages->currentIndex() == 0, "Selecting an icon must not open it.")) return 1;
    desktopIcons->itemDoubleClicked(desktopIcons->item(0));
    if (!require(pages->currentIndex() == 1, "Double-clicking Files must open Files.")) return 1;
    desktopButton->click();
    if (!require(pages->currentIndex() == 0, "Desktop button must return to the workspace.")) return 1;

    auto *start = window.findChild<QPushButton *>(QStringLiteral("startButton"));
    auto *menu = window.findChild<QWidget *>(QStringLiteral("startMenuPanel"));
    auto *search = window.findChild<QLineEdit *>(QStringLiteral("applicationSearch"));
    if (!require(start && menu && search, "Start controls must exist.")) return 1;
    start->click();
    if (!require(start->isChecked() && menu->isVisible(), "Start must open the menu.")) return 1;
    QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(search, &escape);
    if (!require(!start->isChecked() && menu->isHidden(), "Escape in search must dismiss Start.")) return 1;
    start->click();
    QMouseEvent outside(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(desktopCanvas, &outside);
    if (!require(!start->isChecked() && menu->isHidden(), "A desktop click must dismiss Start.")) return 1;

    // Test live theme switching, including widget and rendering policy.
    auto *themes = window.findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
    auto *widgets = window.findChild<QWidget *>(QStringLiteral("neonWidgets"));
    if (!require(themes && themes->count() == 7 && widgets && widgets->isVisible(),
                 "Neon must show widgets on a sufficiently large desktop.")) return 1;
    const QString evidence = qEnvironmentVariable("LINWIN_VISUAL_EVIDENCE_DIR");
    if (!evidence.isEmpty() && !QDir().mkpath(evidence)) return 1;
    for (int i=0; i<themes->count(); ++i) {
        themes->setCurrentIndex(i);
        app.processEvents();
        const QString id=themes->currentData().toString();
        if (!require(settings.value(QStringLiteral("appearance/theme")).toString() == id,
                     "Theme selection must be persisted.")) return 1;
        if (!require(widgets->isVisible() == (id == QStringLiteral("neon")),
                     "Only Neon enables the desktop widgets by default.")) return 1;
        if (id == QStringLiteral("minimal") &&
            !require(!surface->effectsEnabled() && !surface->gpuPreferred() &&
                     surface->property("renderBackend").toString() == QStringLiteral("QPainter solid"),
                     "Minimal must bypass the wallpaper rendering pipeline.")) return 1;
        start->click();
        app.processEvents();
        if (!require(menu->isVisible() && window.findChild<QListWidget *>(QStringLiteral("pinnedApps")),
                     "Every theme must retain the functioning Start menu and pinned list.")) return 1;
        if (!evidence.isEmpty() &&
            !window.grab().save(QDir(evidence).filePath(id + QStringLiteral("-start.png")))) return 1;
        start->click();
        app.processEvents();
        if (!evidence.isEmpty() &&
            !window.grab().save(QDir(evidence).filePath(id + QStringLiteral("-desktop.png")))) return 1;
    }
    themes->setCurrentIndex(0);
    app.processEvents();
    window.resize(480, 600);
    app.processEvents();
    if (!require(more->isVisible() && workspaceButton->isHidden() && desktopButton->isHidden()
                     && clipboard->isHidden() && settingsButton->isHidden()
                     && returnDesktop->isHidden() && closeLinWin->isVisible()
                     && switcher->isHidden() && systemStatus->isHidden(),
                 "Narrow taskbar should move secondary controls into its overflow menu.")) return 1;
    for (auto *widget : {static_cast<QWidget *>(more), static_cast<QWidget *>(clock)}) {
        if (!require(widget->geometry().left() >= 0 && widget->geometry().right() < taskbar->width(),
                     "Visible compact taskbar controls must remain inside the panel.")) return 1;
    }

    const auto actions = more->menu()->actions();
    if (!require(actions.size() == 6 && actions.at(0)->isVisible() && actions.at(1)->isVisible()
                     && actions.at(2)->text() == QStringLiteral("Desktop")
                     && actions.at(3)->text() == QStringLiteral("Switch to Linux desktop")
                     && actions.at(4)->text() == QStringLiteral("Close LinWin")
                     && actions.at(5)->text() == QStringLiteral("Virtual desktops…"),
                 "Narrow taskbar overflow should expose core actions and a virtual desktop entry.")) return 1;
    actions.at(0)->trigger();
    app.processEvents();
    if (!require(pages->currentIndex() == 3, "Overflow Clipboard action should open Clipboard.")) return 1;
    actions.at(1)->trigger();
    app.processEvents();
    if (!require(pages->currentIndex() == 4, "Overflow Settings action should open Settings.")) return 1;

    actions.at(3)->trigger();
    app.processEvents();
    if (!require(window.isMinimized(), "Switch to Linux desktop should minimize LinWin without closing it.")) return 1;
    window.show();
    app.processEvents();
    actions.at(4)->trigger();
    app.processEvents();
    if (!require(window.isHidden(), "Close LinWin should exit the overlay window.")) return 1;

    MainWindow regularWindow(false, false);
    if (!require(!regularWindow.windowFlags().testFlag(Qt::FramelessWindowHint),
                 "The development window option should retain normal window decorations.")) return 1;

    settings.setValue(QStringLiteral("appearance/independentWindows"), true);
    MainWindow floating(true, false);
    floating.resize(1180,760);
    floating.show();
    app.processEvents();
    auto *icons = floating.findChild<QListWidget *>(QStringLiteral("desktopIcons"));
    auto *files = floating.findChild<QDialog *>(QStringLiteral("linwinToolWindow_1"));
    auto *tasks = floating.findChild<QDialog *>(QStringLiteral("linwinToolWindow_2"));
    auto *floatingPages = floating.findChild<QStackedWidget *>(QStringLiteral("shellPages"));
    if (!require(icons && files && tasks && floatingPages && !files->isVisible() && !tasks->isVisible(),
                 "Independent native tools must begin closed.")) return 1;
    icons->itemDoubleClicked(icons->item(0));
    icons->itemDoubleClicked(icons->item(1));
    app.processEvents();
    if (!require(files->isVisible() && tasks->isVisible() && files->isWindow() && tasks->isWindow()
                 && floatingPages->currentIndex()==0, "Two tools must coexist over the desktop.")) return 1;
    if (!evidence.isEmpty()) {
        if (!files->grab().save(QDir(evidence).filePath(QStringLiteral("native-file-explorer.png")))
            || !tasks->grab().save(QDir(evidence).filePath(QStringLiteral("native-task-manager.png")))) return 1;
    }
    files->close();
    app.processEvents();
    if (!require(!files->isVisible() && tasks->isVisible(), "Closing a tool must not close the other.")) return 1;
    icons->itemDoubleClicked(icons->item(0));
    app.processEvents();
    if (!require(files->isVisible(), "A closed tool must be reopenable.")) return 1;
    floating.findChild<QPushButton *>(QStringLiteral("desktopButton"))->click();
    if (!require(!files->isVisible() && !tasks->isVisible(), "Show desktop must hide native tools.")) return 1;
    floating.close();
    return 0;
}