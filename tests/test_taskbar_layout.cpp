// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "desktop_surface.h"
#include "snap_controller.h"
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
#include <QToolButton>
#include <QShortcut>
#include <QTimer>
#include <QProgressBar>
#include "shell_icons.h"
#include <QCheckBox>
#include <QTemporaryDir>
#include <QFile>
#include <QElapsedTimer>
#include <QTextStream>
#include <algorithm>
#include <array>

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

    start->click();
    auto *recent=window.findChild<QListWidget *>(QStringLiteral("recentDocuments"));
    auto *toggle=window.findChild<QCheckBox *>(QStringLiteral("linwinRecentToggle"));
    auto *quickFiles=window.findChild<QPushButton *>(QStringLiteral("linwinStartTool_1"));
    if (!require(recent && toggle && quickFiles && recent->isVisible(), "Start must expose recent documents and quick navigation.")) return 1;
    toggle->setChecked(false);
    if (!require(recent->isHidden() && !settings.value(QStringLiteral("launcher/showRecent")).toBool(), "Recent privacy toggle must persist and hide the list.")) return 1;
    toggle->setChecked(true);
    quickFiles->click();
    if (!require(pages->currentIndex()==1 && menu->isHidden(), "Start Files action must open Files and dismiss Start.")) return 1;
    desktopButton->click();
    // Verify the original icon family and accessible native metric cards.
    auto *iconCache=LinWinVisual::shellIconCache();
    if(!require(iconCache,"Shell icon cache must be application-owned.")) return 1;
    for (auto glyph : {LinWinVisual::Glyph::Files,LinWinVisual::Glyph::Tasks,LinWinVisual::Glyph::Settings,LinWinVisual::Glyph::Clipboard,LinWinVisual::Glyph::Desktop,LinWinVisual::Glyph::Search,LinWinVisual::Glyph::Snap,LinWinVisual::Glyph::More}) {
        const auto icon=LinWinVisual::shellIcon(glyph);
        if (!require(!icon.isNull() && icon.availableSizes().size()==4 && !icon.pixmap(48,48).isNull(),
                     "Every shell glyph must supply four native raster sizes.")) return 1;
    }
    const auto before=iconCache->rasterizations;
    for(int i=0;i<1000;++i) {
        const auto glyph=static_cast<LinWinVisual::Glyph>(i%8);
        const auto icon=LinWinVisual::shellIcon(glyph);
        if(!require(!icon.isNull() && icon.cacheKey()==iconCache->icons[static_cast<unsigned>(glyph)].cacheKey(),
                    "Repeated glyph requests must reuse the native cache.")) return 1;
    }
    if(!require(before==8 && iconCache->rasterizations==8,"1000 warmed icon requests must perform zero new rasterizations.")) return 1;
    for (int index=0;index<3;++index) {
        auto *bar=window.findChild<QProgressBar *>(QStringLiteral("linwinMetricBar_%1").arg(index));
        if (!require(bar && bar->minimum()==0 && bar->maximum()==100 && !bar->accessibleName().isEmpty(),
                     "Live metric bars must be bounded and accessible.")) return 1;
    }
    // Test live theme switching, including widget and rendering policy.
    // Capture the real visual effects with the deterministic CPU fallback in offscreen tests.
    surface->setEffectsEnabled(true);
    surface->setGpuPreferred(false);
    auto *themes = window.findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
    auto *widgets = window.findChild<QWidget *>(QStringLiteral("neonWidgets"));
    if (!require(themes && themes->count() == 7 && widgets && widgets->isVisible(),
                 "Neon must show widgets on a sufficiently large desktop.")) return 1;
    const QString evidence = qEnvironmentVariable("LINWIN_VISUAL_EVIDENCE_DIR");
    if (!evidence.isEmpty() && !QDir().mkpath(evidence)) return 1;
    if(!evidence.isEmpty()) {
        constexpr int iterations=128;
        std::array<qint64,5> uncached{},cached{};
        quint64 checksum=0;
        auto measure=[&](bool useCache){
            QElapsedTimer timer;timer.start();
            for(int i=0;i<iterations;++i) {
                const auto glyph=static_cast<LinWinVisual::Glyph>(i%8);
                const auto icon=useCache?LinWinVisual::shellIcon(glyph):LinWinVisual::rasterizeShellIcon(glyph);
                checksum+=static_cast<quint64>(icon.cacheKey());
            }
            return timer.nsecsElapsed();
        };
        for(std::size_t i=0;i<uncached.size();++i) {
            if(i%2==0){uncached[i]=measure(false);cached[i]=measure(true);}
            else{cached[i]=measure(true);uncached[i]=measure(false);}
        }
        std::sort(uncached.begin(),uncached.end());std::sort(cached.begin(),cached.end());
        if(!require(uncached[2]>0 && cached[2]>0 && iconCache->rasterizations==8,"Benchmark must preserve eight cached glyphs.")) return 1;
        QFile report(QDir(evidence).filePath(QStringLiteral("../native-icon-benchmark.txt")));
        if(!require(report.open(QIODevice::WriteOnly|QIODevice::Text),"Icon benchmark evidence must be writable.")) return 1;
        QTextStream out(&report);
        out<<"Native Qt icon benchmark, 128 requests per trial, 5 alternating trials\n"
           <<"Uncached median ns: "<<uncached[2]<<"\nCached median ns: "<<cached[2]
           <<"\nSpeedup: "<<static_cast<double>(uncached[2])/static_cast<double>(cached[2])
           <<"x\nWarmed glyph rasterizations: "<<iconCache->rasterizations
           <<"\nNew rasterizations for 1000 repeated requests: 0\nChecksum: "<<checksum<<"\n";
    }
    auto *paletteShortcut=window.findChild<QShortcut *>(QStringLiteral("linwinCommandPaletteShortcut"));
    if(!require(paletteShortcut,"Command palette shortcut must exist.")) return 1;
    bool paletteOk=false;
    QTimer::singleShot(0,&window,[&]{
        auto *palette=window.findChild<QDialog *>(QStringLiteral("linwinCommandPalette"));
        auto *search=palette?palette->findChild<QLineEdit *>(QStringLiteral("linwinCommandSearch")):nullptr;
        auto *results=palette?palette->findChild<QListWidget *>(QStringLiteral("linwinCommandResults")):nullptr;
        if(search && results) {
            search->setText(QStringLiteral("impossible-unmatched-query"));
            paletteOk=results->count()==0;
            QMetaObject::invokeMethod(search,"returnPressed",Qt::DirectConnection);
            search->setText(QStringLiteral("theme minimal"));
            paletteOk=paletteOk && results->count()==1 && results->currentItem()!=nullptr;
            if(paletteOk && !evidence.isEmpty()) paletteOk=palette->grab().save(QDir(evidence).filePath(QStringLiteral("native-command-palette.png")));
            if(paletteOk) QMetaObject::invokeMethod(search,"returnPressed",Qt::DirectConnection);
            else palette->reject();
        } else if(palette) palette->reject(); else QApplication::closeAllWindows();
    });
    QMetaObject::invokeMethod(paletteShortcut,"activated",Qt::DirectConnection);
    if(!require(paletteOk && themes->currentData().toString()==QStringLiteral("minimal"),"Palette must filter and execute the selected theme.")) return 1;
    themes->setCurrentIndex(themes->findData(QStringLiteral("neon")));
    auto *quickShortcut=window.findChild<QShortcut *>(QStringLiteral("linwinQuickControlsShortcut"));
    if (!require(quickShortcut,"Quick controls shortcut must exist.")) return 1;
    bool quickOk=false;
    QTimer::singleShot(0,&window,[&]{
        auto *panel=window.findChild<QDialog *>(QStringLiteral("linwinQuickControls"));
        auto *choice=panel?panel->findChild<QComboBox *>(QStringLiteral("linwinQuickTheme")):nullptr;
        auto *effects=panel?panel->findChild<QCheckBox *>(QStringLiteral("linwinQuickEffects")):nullptr;
        auto *previews=panel?panel->findChild<QCheckBox *>(QStringLiteral("linwinQuickPreviews")):nullptr;
        if(choice && effects && previews) {
            choice->setCurrentIndex(choice->findData(QStringLiteral("minimal")));
            quickOk=!effects->isEnabled() && themes->currentData().toString()==QStringLiteral("minimal") && !surface->effectsEnabled();
            choice->setCurrentIndex(choice->findData(QStringLiteral("neon")));
            previews->setChecked(false);
            quickOk=quickOk && !settings.value(QStringLiteral("appearance/taskViewPreviews")).toBool();
            previews->setChecked(true);
            if(quickOk && !evidence.isEmpty()) quickOk=panel->grab().save(QDir(evidence).filePath(QStringLiteral("native-quick-controls.png")));
        }
        if(panel) panel->reject(); else QApplication::closeAllWindows();
    });
    QMetaObject::invokeMethod(quickShortcut,"activated",Qt::DirectConnection);
    if(!require(quickOk,"Quick controls must synchronize live themes and privacy preferences.")) return 1;
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
    auto *overviewShortcut=floating.findChild<QShortcut *>(QStringLiteral("linwinOverviewShortcut"));
    if (!require(overviewShortcut,"Task View keyboard control must exist.")) return 1;
    bool overviewOk=false;
    QTimer::singleShot(0,&floating,[&]{
        auto *overview=floating.findChild<QDialog *>(QStringLiteral("linwinWindowOverview"));
        auto *cards=overview?overview->findChild<QListWidget *>(QStringLiteral("linwinWindowCards")):nullptr;
        overviewOk=cards && cards->count()==2 && cards->item(0)->data(Qt::UserRole).toString()==QStringLiteral("tool:1")
            && cards->item(1)->data(Qt::UserRole).toString()==QStringLiteral("tool:2");
        if (overviewOk && !evidence.isEmpty()) overviewOk=overview->grab().save(QDir(evidence).filePath(QStringLiteral("native-task-view.png")));
        if(overview) overview->reject();
        else QApplication::closeAllWindows();
    });
    QMetaObject::invokeMethod(overviewShortcut,"activated",Qt::DirectConnection);
    if (!require(overviewOk,"Task View must list the two real open native tools.")) return 1;
    auto *snap=files->findChild<QToolButton *>(QStringLiteral("linwinSnapMenu"));
    if (!require(snap && snap->menu() && snap->menu()->actions().size()==9,
                 "Each native tool must expose seven snap zones and restore.")) return 1;
    if (!evidence.isEmpty()) {
        if (!files->grab().save(QDir(evidence).filePath(QStringLiteral("native-file-explorer.png")))
            || !tasks->grab().save(QDir(evidence).filePath(QStringLiteral("native-task-manager.png")))) return 1;
    }
    QPushButton *fileTaskbar=nullptr;
    for(auto *button:floating.findChildren<QPushButton *>())
        if(button->text()==QStringLiteral("Files") && button->objectName()==QStringLiteral("navigationButton")) fileTaskbar=button;
    auto *minAction=fileTaskbar?fileTaskbar->findChild<QAction *>(QStringLiteral("linwinToolMinimize")):nullptr;
    auto *openAction=fileTaskbar?fileTaskbar->findChild<QAction *>(QStringLiteral("linwinToolOpen")):nullptr;
    if (!require(minAction && openAction && minAction->isEnabled(),"Open tools must expose native taskbar actions.")) return 1;
    minAction->trigger();
    app.processEvents();
    if(!require(files->isMinimized(),"Taskbar minimize must minimize the native tool.")) return 1;
    openAction->trigger();
    app.processEvents();
    if(!require(files->isVisible() && !files->isMinimized(),"Taskbar restore must reopen the native tool.")) return 1;
    files->close();
    app.processEvents();
    if (!require(!files->isVisible() && tasks->isVisible(), "Closing a tool must not close the other.")) return 1;
    icons->itemDoubleClicked(icons->item(0));
    app.processEvents();
    if (!require(files->isVisible(), "A closed tool must be reopenable.")) return 1;
    floating.findChild<QPushButton *>(QStringLiteral("desktopButton"))->click();
    if (!require(!files->isVisible() && !tasks->isVisible(), "Show desktop must hide native tools.")) return 1;
    const QRect filesPosition=files->geometry(),tasksPosition=tasks->geometry();
    floating.findChild<QPushButton *>(QStringLiteral("desktopButton"))->click();
    app.processEvents();
    if (!require(files->isVisible() && tasks->isVisible() && files->geometry()==filesPosition && tasks->geometry()==tasksPosition,
                 "Restore workspace must recover both windows without recentering.")) return 1;
    auto *floatingQuick=floating.findChild<QShortcut *>(QStringLiteral("linwinQuickControlsShortcut"));
    if(!require(floatingQuick,"Floating quick controls must exist.")) return 1;
    bool savedOk=false;
    QTimer::singleShot(0,&floating,[&]{
        auto *panel=floating.findChild<QDialog *>(QStringLiteral("linwinQuickControls"));
        auto *save=panel?panel->findChild<QPushButton *>(QStringLiteral("linwinQuickSaveWorkspace")):nullptr;
        if(save) {
            save->click();
            savedOk=settings.contains(QStringLiteral("workspace/profiles/v1/1"));
        }
        if(panel) panel->reject(); else QApplication::closeAllWindows();
    });
    QMetaObject::invokeMethod(floatingQuick,"activated",Qt::DirectConnection);
    if(!require(savedOk,"Quick controls must save a persistent workspace.")) return 1;
    files->close();tasks->close();app.processEvents();
    QTimer::singleShot(0,&floating,[&]{
        auto *panel=floating.findChild<QDialog *>(QStringLiteral("linwinQuickControls"));
        auto *restore=panel?panel->findChild<QPushButton *>(QStringLiteral("linwinQuickRestoreWorkspace")):nullptr;
        if(restore) restore->click();
        else if(panel) panel->reject();
        else QApplication::closeAllWindows();
    });
    QMetaObject::invokeMethod(floatingQuick,"activated",Qt::DirectConnection);
    if(!require(files->isVisible() && tasks->isVisible(),"Saved profile must restore both native tools.")) return 1;
    const QRect finalFilesGeometry=files->geometry();
    floating.close();
    app.processEvents();
    if(!require(settings.value(QStringLiteral("workspace/toolGeometry/v1/1")).toRect()==finalFilesGeometry,
                 "Closing LinWin must preserve the last valid tool geometry.")) return 1;
    MainWindow reopenedShell(true,false);
    reopenedShell.resize(1180,760);reopenedShell.show();app.processEvents();
    auto *reopenedIcons=reopenedShell.findChild<QListWidget *>(QStringLiteral("desktopIcons"));
    auto *reopenedFiles=reopenedShell.findChild<QDialog *>(QStringLiteral("linwinToolWindow_1"));
    if(!require(reopenedIcons && reopenedFiles && !reopenedFiles->isVisible(),"Recall must not auto-launch tools.")) return 1;
    reopenedIcons->itemDoubleClicked(reopenedIcons->item(0));app.processEvents();
    if(!require(reopenedFiles->isVisible() && reopenedFiles->size()==finalFilesGeometry.size(),
                 "Reopened shell must recover the saved tool size.")) return 1;
    reopenedShell.close();
    // Exercise the real controller on a small native dialog independently of
    // the content minimum sizes of the full File Explorer and Task Manager.
    QDialog probe;
    probe.resize(240,180);
    probe.move(50,50);
    auto *controller=new SnapController(&probe,nullptr);
    auto *layout=new QVBoxLayout(&probe);
    layout->addWidget(controller->createButton());
    probe.show();
    app.processEvents();
    const QRect original=probe.geometry();
    if (!require(controller->snap(LinWinSnap::Zone::Left), "Live left snap must succeed.")) return 1;
    app.processEvents();
    if (!require(probe.width()>original.width() && probe.height()>original.height(),
                 "Live snap must resize the native dialog.")) return 1;
    controller->restore();
    app.processEvents();
    if (!require(probe.size()==original.size(), "Restore must recover the previous native window size.")) return 1;
    probe.close();
    // Use a harmless fake wmctrl only in this isolated test process.
    QTemporaryDir fixture;
    if(!require(fixture.isValid(),"Window polling fixture directory must exist.")) return 1;
    QFile fake(fixture.filePath(QStringLiteral("wmctrl")));
    if(!require(fake.open(QIODevice::WriteOnly),"Fake wmctrl must be writable.")) return 1;
    fake.write("#!/bin/sh\nexit 0\n");fake.close();
    if(!require(fake.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner),"Fake wmctrl must be executable.")) return 1;
    qputenv("PATH",fixture.path().toLocal8Bit()+":"+qgetenv("PATH"));
    qputenv("XDG_SESSION_TYPE","x11");qunsetenv("WAYLAND_DISPLAY");
    MainWindow pollingShell(true,false);
    auto *windowPoll=pollingShell.findChild<QTimer *>(QStringLiteral("linwinExternalWindowPoll"));
    auto *workspacePoll=pollingShell.findChild<QTimer *>(QStringLiteral("linwinExternalWorkspacePoll"));
    if(!require(windowPoll&&workspacePoll&&!windowPoll->isActive()&&!workspacePoll->isActive(),"Hidden shell must not run external polling timers.")) return 1;
    pollingShell.show();app.processEvents();
    if(!require(windowPoll->isActive()&&workspacePoll->isActive(),"Visible shell must resume external polling.")) return 1;
    pollingShell.showMinimized();app.processEvents();
    if(!require(!windowPoll->isActive()&&!workspacePoll->isActive(),"Minimized shell must stop both external polling timers.")) return 1;
    pollingShell.showNormal();app.processEvents();
    if(!require(windowPoll->isActive()&&workspacePoll->isActive(),"Restored shell must resume both polling timers.")) return 1;
    pollingShell.close();app.processEvents();
    if(!require(!windowPoll->isActive()&&!workspacePoll->isActive(),"Closed shell must stop both polling timers.")) return 1;
    return 0;
}