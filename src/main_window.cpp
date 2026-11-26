// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "workspace_profiles.h"
#include "command_palette.h"
#include "snap_controller.h"
#include "desktop_surface.h"
#include "theme_visuals.h"
#include "shell_icons.h"
#include "window_overview.h"
#include "desktop_widgets.h"

#include "app_launcher_widget.h"
#include "clipboard_history_widget.h"
#include "file_manager_widget.h"
#include "system_profile.h"
#include "settings_hub_widget.h"
#include "task_manager_widget.h"
#include "text_scale.h"

#include <QApplication>
#include <QDateTime>
#include <QCloseEvent>
#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QScreen>
#include <QEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QListWidget>
#include <QUrl>
#include <algorithm>
#include <QCoreApplication>
#include <QKeySequence>
#include <QProcess>
#include <QRegularExpression>
#include <QShortcut>
#include <QStandardPaths>
#include <QSettings>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

namespace {
QFrame *makeCard(const QString &title, const QString &body, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("card"));
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    auto *heading = new QLabel(title, card);
    heading->setObjectName(QStringLiteral("cardTitle"));
    heading->setWordWrap(true);
    auto *description = new QLabel(body, card);
    description->setObjectName(QStringLiteral("cardBody"));
    description->setWordWrap(true);
    layout->addWidget(heading);
    layout->addWidget(description);
    layout->addStretch();
    return card;
}

QPushButton *makePanelButton(const QString &text, const QString &name, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setObjectName(name);
    button->setCheckable(true);
    button->setFocusPolicy(Qt::StrongFocus);
    return button;
}
}

MainWindow::MainWindow(bool overlayMode, bool shellSessionMode, QWidget *parent)
    : QMainWindow(parent), overlayMode_(overlayMode), shellSessionMode_(shellSessionMode)
{
    setWindowTitle(QStringLiteral("LinWin Desktop"));
    setMinimumSize(480, 420);
    if (overlayMode_) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    } else {
        resize(1180, 760);
    }

    const SystemProfile profile = SystemProfile::detect();
    auto *root = new QWidget(this);
    root->setObjectName(QStringLiteral("shellRoot"));
    auto *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    desktopArea_ = new QWidget(root);
    desktopArea_->setObjectName(QStringLiteral("desktopArea"));
    rootLayout->addWidget(desktopArea_, 1);

    pages_ = new QStackedWidget(desktopArea_);
    pages_->setObjectName(QStringLiteral("shellPages"));

    auto *scroll = new QScrollArea(pages_);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scroll);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(18);

    auto *eyebrow = new QLabel(QStringLiteral("CROSS-DISTRO DESKTOP TOOLS"), content);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    auto *title = new QLabel(QStringLiteral("A familiar desktop, built for Linux."), content);
    title->setObjectName(QStringLiteral("pageTitle"));
    title->setWordWrap(true);
    auto *intro = new QLabel(
        QStringLiteral("LinWin brings familiar desktop features into one shell. The finished product aims to handle setup and everyday use without terminal or Bash knowledge."), content);
    intro->setObjectName(QStringLiteral("intro"));
    intro->setWordWrap(true);
    layout->addWidget(eyebrow);
    layout->addWidget(title);
    layout->addWidget(intro);

    auto *systemHeading = new QLabel(QStringLiteral("This system"), content);
    systemHeading->setObjectName(QStringLiteral("sectionTitle"));
    layout->addWidget(systemHeading);
    auto *systemGrid = new QGridLayout;
    systemGrid->setSpacing(12);
    systemGrid->addWidget(makeCard(QStringLiteral("Distribution"), profile.distribution, content), 0, 0);
    systemGrid->addWidget(makeCard(QStringLiteral("Desktop"), profile.desktopEnvironment, content), 0, 1);
    systemGrid->addWidget(makeCard(QStringLiteral("Session"), profile.sessionType, content), 1, 0);
    systemGrid->addWidget(makeCard(QStringLiteral("Kernel"), profile.kernel, content), 1, 1);
    systemGrid->setColumnStretch(0, 1);
    systemGrid->setColumnStretch(1, 1);
    layout->addLayout(systemGrid);

    auto *statusHeading = new QLabel(QStringLiteral("Project status"), content);
    statusHeading->setObjectName(QStringLiteral("sectionTitle"));
    auto *status = new QLabel(shellSessionMode_ ? QStringLiteral("LinWin desktop session is active")
                                        : QStringLiteral("LinWin desktop shell preview"), content);
    status->setObjectName(QStringLiteral("statusBadge"));
    auto *statusDescription = new QLabel(shellSessionMode_
        ? QStringLiteral("You are using LinWin as an X11 desktop session with the XFCE window manager. Your previous desktop remains available in the login session selector. Choose Sign out to return to that selector.")
        : QStringLiteral("The taskbar, searchable Start menu, file manager, and task manager are still evolving. Launch LinWin Setup to check whether this system can run the current XFCE/X11 session integration."), content);
    statusDescription->setObjectName(QStringLiteral("intro"));
    statusDescription->setWordWrap(true);
    layout->addWidget(statusHeading);
    layout->addWidget(status);
    layout->addWidget(statusDescription);

    auto *compatibilityHeading = new QLabel(QStringLiteral("Designed for choice"), content);
    compatibilityHeading->setObjectName(QStringLiteral("sectionTitle"));
    auto *compatibility = new QLabel(
        QStringLiteral("The plan is to support Linux Mint Cinnamon, Xfce, and MATE, plus KDE Plasma and additional desktops through dedicated integrations."), content);
    compatibility->setObjectName(QStringLiteral("intro"));
    compatibility->setWordWrap(true);
    layout->addWidget(compatibilityHeading);
    layout->addWidget(compatibility);
    layout->addStretch();
    scroll->setWidget(content);

    if (overlayMode_) {
        auto *desktop = new DesktopSurface(pages_);
        desktopSurface_ = desktop;
        auto *desktopLayout = new QVBoxLayout(desktop);
        desktopLayout->setContentsMargins(10, 10, 10, 10);
        auto *icons = new QListWidget(desktop);
        icons->setObjectName(QStringLiteral("desktopIcons"));
        icons->setViewMode(QListView::IconMode);
        icons->setFlow(QListView::TopToBottom);
        icons->setWrapping(true);
        icons->setResizeMode(QListView::Adjust);
        icons->setMovement(QListView::Static);
        icons->setGridSize(QSize(96, 88));
        icons->setIconSize(QSize(42, 42));
        icons->setSelectionMode(QAbstractItemView::ExtendedSelection);
        icons->setFrameShape(QFrame::NoFrame);
        icons->setAutoFillBackground(false);
        icons->viewport()->setAutoFillBackground(false);
        icons->viewport()->setAttribute(Qt::WA_TranslucentBackground);
        icons->setWordWrap(true);
        icons->setContextMenuPolicy(Qt::CustomContextMenu);
        auto *desktopRow = new QHBoxLayout;
        desktopRow->addWidget(icons, 1);
        desktopWidgets_ = new DesktopWidgets(desktop);
        desktopRow->addWidget(desktopWidgets_);
        desktopLayout->addLayout(desktopRow, 1);
        const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        const auto refreshIcons = [this, icons, desktopPath]() {
            icons->clear();
            const auto addShortcut = [icons](const QString &name, LinWinVisual::Glyph icon, int page) {
                auto *item = new QListWidgetItem(LinWinVisual::shellIcon(icon), name, icons);
                item->setData(Qt::UserRole, page);
                item->setTextAlignment(Qt::AlignHCenter);
            };
            addShortcut(tr("Files"), LinWinVisual::Glyph::Files, 1);
            addShortcut(tr("Task Manager"), LinWinVisual::Glyph::Tasks, 2);
            addShortcut(tr("Settings"), LinWinVisual::Glyph::Settings, 4);
            if (desktopPath.isEmpty() || !QDir(desktopPath).exists()) return;
            QFileIconProvider provider;
            const auto entries = QDir(desktopPath).entryInfoList(
                QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
            for (const QFileInfo &entry : entries) {
                auto *item = new QListWidgetItem(provider.icon(entry), entry.fileName(), icons);
                item->setData(Qt::UserRole + 1, entry.absoluteFilePath());
                item->setToolTip(entry.absoluteFilePath());
                item->setTextAlignment(Qt::AlignHCenter);
            }
        };
        const auto openIcon = [this](QListWidgetItem *item) {
            if (item == nullptr) return;
            if (item->data(Qt::UserRole).isValid()) {
                showPage(item->data(Qt::UserRole).toInt());
                return;
            }
            const QString path = item->data(Qt::UserRole + 1).toString();
            if (path.isEmpty() || !QFileInfo::exists(path)) {
                QMessageBox::information(this, tr("Desktop"), tr("This item is no longer available."));
                return;
            }
            const QFileInfo info(path);
            if (info.isExecutable() && !info.isDir()) {
                QMessageBox::information(this, tr("Desktop"),
                    tr("Open executable files using your trusted file manager."));
                return;
            }
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
                QMessageBox::warning(this, tr("Desktop"), tr("No application could open this item."));
        };
        connect(icons, &QListWidget::itemDoubleClicked, this, openIcon);
        for (const auto key : {Qt::Key_Return, Qt::Key_Enter}) {
            auto *openShortcut = new QShortcut(QKeySequence(key), icons);
            openShortcut->setContext(Qt::WidgetShortcut);
            connect(openShortcut, &QShortcut::activated, this, [icons, openIcon]() {
                openIcon(icons->currentItem());
            });
        }
        auto *refreshShortcut = new QShortcut(QKeySequence(Qt::Key_F5), icons);
        refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(refreshShortcut, &QShortcut::activated, this, refreshIcons);
        connect(icons, &QListWidget::customContextMenuRequested, this,
                [this, desktop, icons, openIcon, refreshIcons, desktopPath](const QPoint &position) {
            QMenu menu(this);
            if (auto *item = icons->itemAt(position)) {
                icons->setCurrentItem(item);
                menu.addAction(tr("Open"), this, [openIcon, icons]() { openIcon(icons->currentItem()); });
                menu.addSeparator();
            }
            menu.addAction(tr("Refresh"), this, refreshIcons);
            menu.addAction(tr("Task View (Ctrl+Shift+F9)"),this,&MainWindow::showWindowOverview);
            menu.addAction(tr("Quick controls (Ctrl+Shift+F10)"),this,&MainWindow::showQuickControls);
            menu.addAction(tr("Command Palette (Ctrl+Shift+P)"),this,&MainWindow::showCommandPalette);
            if (!toolWindows_.isEmpty()) {
                auto *restore=menu.addAction(tr("Restore hidden workspace"),this,&MainWindow::restoreHiddenWorkspace);
                restore->setEnabled(!hiddenWorkspace_.isEmpty());
                auto *profiles=menu.addMenu(tr("Workspace profiles"));
                QSettings settings;
                for(int slot=1;slot<=3;++slot) {
                    auto *profile=profiles->addMenu(tr("Workspace %1").arg(slot));
                    profile->addAction(tr("Save open tools"),this,[this,slot]{saveWorkspaceProfile(slot);});
                    auto *load=profile->addAction(tr("Restore saved tools"),this,[this,slot]{loadWorkspaceProfile(slot);});
                    load->setEnabled(!LinWinWorkspace::load(settings,slot).isEmpty());
                }
            }
            auto *visual = menu.addMenu(tr("Visual effects"));
            auto *glow = visual->addAction(tr("Desktop lighting"));
            glow->setCheckable(true);
            glow->setChecked(desktop->effectsEnabled());
            connect(glow, &QAction::toggled, desktop, &DesktopSurface::setEffectsEnabled);
            auto *gpu = visual->addAction(tr("Prefer OpenGL rendering"));
            gpu->setCheckable(true);
            gpu->setChecked(desktop->gpuPreferred());
            connect(gpu, &QAction::toggled, desktop, &DesktopSurface::setGpuPreferred);
            if (!desktopPath.isEmpty() && QDir(desktopPath).exists()) {
                menu.addAction(tr("Open desktop folder"), this, [desktopPath]() {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(desktopPath));
                });
            }
            menu.addSeparator();
            menu.addAction(tr("Settings"), this, &MainWindow::showSettingsHub);
            if (!shellSessionMode_)
                menu.addAction(tr("Return to Linux desktop"), this, &MainWindow::returnToLinuxDesktop);
            menu.exec(icons->viewport()->mapToGlobal(position));
        });
        refreshIcons();
        if (!desktopPath.isEmpty() && QDir(desktopPath).exists()) {
            auto *watcher = new QFileSystemWatcher(icons);
            watcher->addPath(desktopPath);
            connect(watcher, &QFileSystemWatcher::directoryChanged, this,
                    [icons, refreshIcons](const QString &) {
                // Do not invalidate item pointers while a desktop context menu is running.
                if (QApplication::activePopupWidget() == nullptr) refreshIcons();
            });
        }
        pages_->addWidget(desktop);
    } else {
        scroll->hide();
        pages_->addWidget(scroll);
    }
    fileManager_ = new FileManagerWidget(pages_);
    pages_->addWidget(fileManager_);
    taskManager_ = new TaskManagerWidget(pages_);
    pages_->addWidget(taskManager_);
    clipboardHistory_ = new ClipboardHistoryWidget(pages_);
    pages_->addWidget(clipboardHistory_);
    settingsHub_ = new SettingsHubWidget(pages_);
    pages_->addWidget(settingsHub_);
    if (overlayMode_ && QSettings().value(QStringLiteral("appearance/independentWindows"), true).toBool())
        initializeToolWindows();
    connect(settingsHub_, &SettingsHubWidget::componentEnabledChanged,
            this, &MainWindow::applyComponentVisibility);
    connect(settingsHub_, &SettingsHubWidget::textScaleChanged,
            this, &MainWindow::applyTextScale);

    startMenu_ = new AppLauncherWidget(desktopArea_);
    startMenu_->setObjectName(QStringLiteral("startMenuPanel"));
    startMenu_->setWindowFlags(Qt::Widget);
    startMenu_->hide();
    connect(startMenu_,&AppLauncherWidget::toolRequested,this,&MainWindow::showPage);

    auto *taskbar = new QFrame(root);
    taskbar->setObjectName(QStringLiteral("taskbar"));
    taskbar->setFixedHeight(44);
    auto *taskbarLayout = new QHBoxLayout(taskbar);
    taskbarLayout->setContentsMargins(3, 2, 3, 2);
    taskbarLayout->setSpacing(2);

    startButton_ = makePanelButton(QStringLiteral("LinWin"), QStringLiteral("startButton"), taskbar);
    startButton_->setToolTip(tr("Open the app menu (Super or Ctrl+Esc)"));
    startButton_->setIcon(LinWinVisual::logoIcon());
    startButton_->setIconSize(QSize(28,28));
    setWindowIcon(LinWinVisual::logoIcon());
    homeButton_ = makePanelButton(QStringLiteral("Desktop"), QStringLiteral("desktopButton"), taskbar);
    filesButton_ = makePanelButton(QStringLiteral("Files"), QStringLiteral("navigationButton"), taskbar);
    tasksButton_ = makePanelButton(QStringLiteral("Tasks"), QStringLiteral("navigationButton"), taskbar);
    tasksButton_->setToolTip(tr("Open Task Manager (Ctrl+Shift+Esc)"));
    clipboardButton_ = makePanelButton(QStringLiteral("Clipboard"), QStringLiteral("navigationButton"), taskbar);
    settingsButton_ = makePanelButton(QStringLiteral("Settings"), QStringLiteral("navigationButton"), taskbar);
    for (const auto &entry : QList<QPair<QPushButton *,LinWinVisual::Glyph>>{{homeButton_,LinWinVisual::Glyph::Desktop},{filesButton_,LinWinVisual::Glyph::Files},{tasksButton_,LinWinVisual::Glyph::Tasks},{clipboardButton_,LinWinVisual::Glyph::Clipboard},{settingsButton_,LinWinVisual::Glyph::Settings}}) {
        entry.first->setIcon(LinWinVisual::shellIcon(entry.second));
        entry.first->setIconSize(QSize(20,20));
    }
    if (overlayMode_) {
        signOutButton_ = makePanelButton(shellSessionMode_ ? tr("Sign out") : tr("Linux desktop"),
                                        QStringLiteral("returnDesktopButton"), taskbar);
        signOutButton_->setToolTip(shellSessionMode_
            ? tr("End the LinWin session and return to the login screen")
            : tr("Switch back to the Linux desktop; LinWin will stay open in the taskbar"));
    }
    closeButton_ = makePanelButton(QStringLiteral("×"), QStringLiteral("closeLinWinButton"), taskbar);
    closeButton_->setToolTip(tr("Close LinWin and return to the Linux desktop"));
    overflowButton_ = makePanelButton(tr("More"), QStringLiteral("taskbarOverflowButton"), taskbar);
    overflowButton_->setToolTip(tr("Open additional shell actions"));
    overflowButton_->setIcon(LinWinVisual::shellIcon(LinWinVisual::Glyph::More));
    overflowButton_->setIconSize(QSize(18,18));
    auto *overflowMenu = new QMenu(overflowButton_);
    clipboardOverflowAction_ = overflowMenu->addAction(tr("Clipboard"));
    settingsOverflowAction_ = overflowMenu->addAction(tr("Settings"));
    desktopOverflowAction_ = overflowMenu->addAction(tr("Desktop"));
    if (overlayMode_) {
        if (shellSessionMode_)
            signOutOverflowAction_ = overflowMenu->addAction(tr("Sign out of LinWin"));
        else
            returnDesktopOverflowAction_ = overflowMenu->addAction(tr("Switch to Linux desktop"));
    }
    closeOverflowAction_ = overflowMenu->addAction(tr("Close LinWin"));
    workspaceOverflowAction_ = overflowMenu->addAction(tr("Virtual desktops…"));
    overflowButton_->setMenu(overflowMenu);
    workspaceMenu_ = new QMenu(this);
    workspaceButton_ = makePanelButton(tr("Desktop 1  ▾"), QStringLiteral("workspaceButton"), taskbar);
    workspaceButton_->setToolTip(tr("Switch or add virtual desktops"));
    workspaceButton_->setMenu(workspaceMenu_);
    homeButton_->setChecked(true);
    for (const QString &component : {QStringLiteral("start-menu"), QStringLiteral("files"),
                                     QStringLiteral("task-manager"), QStringLiteral("clipboard")})
        applyComponentVisibility(component, componentEnabled(component));

    auto *windowScroll = new QScrollArea(taskbar);
    windowSwitcher_ = windowScroll;
    windowScroll->setObjectName(QStringLiteral("windowSwitcher"));
    windowScroll->setWidgetResizable(true);
    windowScroll->setFrameShape(QFrame::NoFrame);
    windowScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    windowScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    windowScroll->setFixedHeight(38);
    auto *windowList = new QWidget(windowScroll);
    windowButtonsLayout_ = new QHBoxLayout(windowList);
    windowButtonsLayout_->setContentsMargins(2, 0, 2, 0);
    windowButtonsLayout_->setSpacing(4);
    windowSwitcherStatus_ = new QLabel(tr("Checking open windows…"), windowList);
    windowSwitcherStatus_->setObjectName(QStringLiteral("windowSwitcherStatus"));
    windowButtonsLayout_->addWidget(windowSwitcherStatus_);
    windowScroll->setWidget(windowList);

    systemStatus_ = new QLabel(
        QStringLiteral("%1  ·  %2").arg(profile.distribution, profile.sessionType), taskbar);
    systemStatus_->setObjectName(QStringLiteral("panelStatus"));
    systemStatus_->setToolTip(tr("Detected Linux distribution and session type"));
    systemStatus_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    clock_ = new QLabel(taskbar);
    clock_->setObjectName(QStringLiteral("panelClock"));
    clock_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    const auto refreshClock = [clock = clock_]() {
        clock->setText(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm\nddd, d MMM")));
    };
    refreshClock();
    auto *clockTimer = new QTimer(this);
    clockTimer->setInterval(30 * 1000);
    connect(clockTimer, &QTimer::timeout, this, refreshClock);
    clockTimer->start();

    taskbarLayout->addWidget(startButton_);
    taskbarLayout->addWidget(homeButton_);
    taskbarLayout->addWidget(filesButton_);
    taskbarLayout->addWidget(tasksButton_);
    taskbarLayout->addWidget(clipboardButton_);
    taskbarLayout->addWidget(settingsButton_);
    if (signOutButton_ != nullptr)
        taskbarLayout->addWidget(signOutButton_);
    taskbarLayout->addWidget(overflowButton_);
    taskbarLayout->addWidget(workspaceButton_);
    taskbarLayout->addWidget(windowScroll, 1);
    taskbarLayout->addWidget(systemStatus_);
    taskbarLayout->addSpacing(14);
    taskbarLayout->addWidget(clock_);
    taskbarLayout->addWidget(closeButton_);
    rootLayout->addWidget(taskbar);
    setCentralWidget(root);
    QApplication::instance()->installEventFilter(this);
    updateTaskbarLayout();

    connect(startButton_, &QPushButton::toggled, this, [this](bool visible) {
        startMenu_->setVisible(visible);
        if (visible) {
            updateShellGeometry();
            startMenu_->raise();
            startMenu_->focusSearch();
        }
    });
    initializeToolTaskbarActions();
    connect(homeButton_, &QPushButton::clicked, this, &MainWindow::toggleWorkspaceVisibility);
    homeButton_->setToolTip(tr("Show desktop or restore hidden tools (Ctrl+Alt+D)"));
    connect(filesButton_, &QPushButton::clicked, this, [this]() { showPage(1); });
    connect(tasksButton_, &QPushButton::clicked, this, [this]() { showPage(2); });
    connect(clipboardButton_, &QPushButton::clicked, this, &MainWindow::showClipboardCenter);
    connect(settingsButton_, &QPushButton::clicked, this, &MainWindow::showSettingsHub);
    connect(clipboardOverflowAction_, &QAction::triggered, this, &MainWindow::showClipboardCenter);
    connect(settingsOverflowAction_, &QAction::triggered, this, &MainWindow::showSettingsHub);
    connect(desktopOverflowAction_, &QAction::triggered, this, [this]() { showPage(0); });
    connect(workspaceOverflowAction_, &QAction::triggered, this, [this]() {
        if (workspaceMenu_ != nullptr && overflowButton_ != nullptr)
            workspaceMenu_->popup(overflowButton_->mapToGlobal(QPoint(0, overflowButton_->height())));
    });
    if (signOutButton_ != nullptr) {
        if (shellSessionMode_) {
            const auto requestSignOut = [this]() {
                if (QMessageBox::question(this, tr("Sign out of LinWin?"),
                        tr("Your applications will close and the login screen will appear. Choose your previous desktop there to return to it."),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
                    close();
            };
            connect(signOutButton_, &QPushButton::clicked, this, requestSignOut);
            connect(signOutOverflowAction_, &QAction::triggered, this, requestSignOut);
        } else {
            connect(signOutButton_, &QPushButton::clicked, this, &MainWindow::returnToLinuxDesktop);
            connect(returnDesktopOverflowAction_, &QAction::triggered, this, &MainWindow::returnToLinuxDesktop);
        }
    }
    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
    connect(closeOverflowAction_, &QAction::triggered, this, &QWidget::close);
    auto *paletteShortcut=new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+P")),this);
    paletteShortcut->setObjectName(QStringLiteral("linwinCommandPaletteShortcut"));
    paletteShortcut->setContext(Qt::ApplicationShortcut);
    connect(paletteShortcut,&QShortcut::activated,this,&MainWindow::showCommandPalette);
    auto *quickControlsShortcut=new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+F10")),this);
    quickControlsShortcut->setObjectName(QStringLiteral("linwinQuickControlsShortcut"));
    quickControlsShortcut->setContext(Qt::ApplicationShortcut);
    connect(quickControlsShortcut,&QShortcut::activated,this,&MainWindow::showQuickControls);
    auto *overviewShortcut=new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+F9")),this);
    overviewShortcut->setObjectName(QStringLiteral("linwinOverviewShortcut"));
    overviewShortcut->setContext(Qt::ApplicationShortcut);
    connect(overviewShortcut,&QShortcut::activated,this,&MainWindow::showWindowOverview);
    auto *workspaceVisibilityShortcut=new QShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+D")),this);
    workspaceVisibilityShortcut->setObjectName(QStringLiteral("linwinWorkspaceVisibilityShortcut"));
    workspaceVisibilityShortcut->setContext(Qt::ApplicationShortcut);
    connect(workspaceVisibilityShortcut,&QShortcut::activated,this,&MainWindow::toggleWorkspaceVisibility);
    auto *settingsShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+,")), this);
    settingsShortcut->setObjectName(QStringLiteral("settingsHubShortcut"));
    settingsShortcut->setContext(Qt::ApplicationShortcut);
    connect(settingsShortcut, &QShortcut::activated, this, &MainWindow::showSettingsHub);
    auto *startShortcut = new QShortcut(QKeySequence(Qt::Key_Super_L), this);
    startShortcut->setObjectName(QStringLiteral("startMenuShortcut"));
    startShortcut->setContext(Qt::ApplicationShortcut);
    connect(startShortcut, &QShortcut::activated, this, [this]() {
        if (componentEnabled(QStringLiteral("start-menu"))) startButton_->toggle();
    });
    auto *startFallbackShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Escape")), this);
    startFallbackShortcut->setObjectName(QStringLiteral("startMenuFallbackShortcut"));
    startFallbackShortcut->setContext(Qt::ApplicationShortcut);
    connect(startFallbackShortcut, &QShortcut::activated, this, [this]() {
        if (componentEnabled(QStringLiteral("start-menu"))) startButton_->toggle();
    });
    if (overlayMode_ && !shellSessionMode_) {
        auto *showLinuxDesktopShortcut = new QShortcut(QKeySequence(QStringLiteral("Meta+D")), this);
        showLinuxDesktopShortcut->setObjectName(QStringLiteral("showLinuxDesktopShortcut"));
        showLinuxDesktopShortcut->setContext(Qt::ApplicationShortcut);
        connect(showLinuxDesktopShortcut, &QShortcut::activated, this, &MainWindow::returnToLinuxDesktop);
    }
    auto *filesShortcut = new QShortcut(QKeySequence(QStringLiteral("Meta+E")), this);
    filesShortcut->setObjectName(QStringLiteral("openFilesShortcut"));
    filesShortcut->setContext(Qt::ApplicationShortcut);
    connect(filesShortcut, &QShortcut::activated, this, [this]() { showPage(1); });
    auto *taskManagerShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+Escape")), this);
    taskManagerShortcut->setObjectName(QStringLiteral("openTaskManagerShortcut"));
    taskManagerShortcut->setContext(Qt::ApplicationShortcut);
    connect(taskManagerShortcut, &QShortcut::activated, this, [this]() { showPage(2); });
    auto *superClipboardShortcut = new QShortcut(QKeySequence(Qt::META | Qt::Key_V), this);
    superClipboardShortcut->setObjectName(QStringLiteral("clipboardSuperVShortcut"));
    superClipboardShortcut->setContext(Qt::ApplicationShortcut);
    connect(superClipboardShortcut, &QShortcut::activated, this, &MainWindow::showClipboardQuickPicker);
    auto *fallbackClipboardShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+V")), this);
    fallbackClipboardShortcut->setObjectName(QStringLiteral("clipboardFallbackShortcut"));
    fallbackClipboardShortcut->setContext(Qt::ApplicationShortcut);
    connect(fallbackClipboardShortcut, &QShortcut::activated, this, &MainWindow::showClipboardQuickPicker);

    const auto addTextScaleShortcut = [this](const QKeySequence &sequence, int delta, const QString &name) {
        auto *shortcut = new QShortcut(sequence, this);
        shortcut->setObjectName(name);
        shortcut->setContext(Qt::ApplicationShortcut);
        connect(shortcut, &QShortcut::activated, this, [this, delta]() {
            settingsHub_->setTextScalePercent(delta == 0 ? 100 : settingsHub_->textScalePercent() + delta);
        });
    };
    addTextScaleShortcut(QKeySequence::ZoomIn, 5, QStringLiteral("increaseTextSizeShortcut"));
    addTextScaleShortcut(QKeySequence::ZoomOut, -5, QStringLiteral("decreaseTextSizeShortcut"));
    addTextScaleShortcut(QKeySequence(QStringLiteral("Ctrl+0")), 0, QStringLiteral("resetTextSizeShortcut"));

    connect(pages_, &QStackedWidget::currentChanged, this, [this](int index) {
        homeButton_->setChecked(index == 0);
        filesButton_->setChecked(index == 1);
        tasksButton_->setChecked(index == 2);
        clipboardButton_->setChecked(index == 3);
        settingsButton_->setChecked(index == 4);
    });

    baseStyleSheet_ = QStringLiteral(
        "QMainWindow, QWidget { background: #f4f6fa; color: #172033; font-family: 'Sans Serif'; }"
        "#desktopArea { background: #f4f6fa; }"
        "#windowsDesktop, #desktopIcons, #desktopIcons QWidget { background: transparent; }"
        "#desktopIcons { background: transparent; color: white; border: none; outline: none; font-size: 12px; }"
        "#desktopIcons::item { background: transparent; border: 1px solid transparent; padding: 3px; }"
        "#desktopIcons::item:hover { background: rgba(220,240,255,35); border-color: rgba(220,240,255,85); }"
        "#desktopIcons::item:selected { background: rgba(135,205,255,95); border-color: rgba(220,240,255,180); color: white; }"
        "#eyebrow { color: #3568d4; font-size: 11px; font-weight: 700; letter-spacing: 1px; }"
        "#pageTitle { color: #111a2b; font-size: 30px; font-weight: 700; }"
        "#sectionTitle { color: #172033; font-size: 18px; font-weight: 650; margin-top: 6px; }"
        "#intro { color: #526078; font-size: 14px; }"
        "#card { background: white; border: 1px solid #e0e6ef; border-radius: 12px; min-height: 78px; }"
        "#cardTitle { color: #526078; font-size: 12px; font-weight: 600; }"
        "#cardBody { color: #172033; font-size: 16px; font-weight: 600; }"
        "#statusBadge { color: #714c06; background: #fff0c2; border-radius: 8px; padding: 8px 12px; font-weight: 600; }"
        "#privacyNote { color: #654d17; background: #fff7dc; border: 1px solid #f0dda1; border-radius: 8px; padding: 10px 12px; }"
        "#clipboardEntries, #applicationList { background: white; border: 1px solid #e0e6ef; border-radius: 8px; padding: 4px; }"
        "#fileView { background: white; border: 1px solid #e0e6ef; border-radius: 8px; alternate-background-color: #f8faff; }"
        "#fileLocation { background: white; border: 1px solid #cfd9e8; border-radius: 7px; padding: 8px 10px; }"
        "#fileAction { background: white; color: #263853; border: 1px solid #d5deeb; border-radius: 7px; padding: 7px 10px; }"
        "#fileAction:hover { background: #edf3ff; border-color: #a9bfeb; }"
        "#fileStatus { color: #64738a; font-size: 12px; }"
        "#processTable { background: white; border: 1px solid #e0e6ef; border-radius: 8px; alternate-background-color: #f8faff; }"
        "#dangerAction { background: #fff1f0; color: #9b2020; border: 1px solid #f0c6c2; border-radius: 6px; padding: 8px 12px; font-weight: 600; }"
        "#dangerAction:hover { background: #ffe1df; }"
        "#secondaryAction { background: white; color: #263853; border: 1px solid #d5deeb; border-radius: 6px; padding: 8px 12px; }"
        "#secondaryAction:hover { background: #edf3ff; }"
        "#taskbar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #222b37,stop:1 #111820); border-top: 1px solid #4b6176; }"
        "#taskbar QPushButton { background: transparent; color: #f2f2f2; border: 0; border-radius: 0px; padding: 5px 8px; font-size: 12px; min-height: 25px; }"
        "#taskbar QPushButton:hover { background: #35465a; border-bottom: 2px solid #82c8ff; }"
        "#taskbar QPushButton:checked { background: #2d4770; color: white; }"
        "#taskbar #startButton { background: transparent; color: white; font-weight: 600; padding-left: 12px; padding-right: 12px; }"
        "#taskbar #startButton:hover { background: #353a42; color: #80caff; }"
        "#windowSwitcherStatus { color: #9dabc2; font-size: 11px; padding: 0 8px; }"
        "#windowSwitcher QPushButton { background: #292d34; color: #f2f2f2; border: 0; border-bottom: 2px solid #64b8f0; border-radius: 0px; padding: 3px 8px; }"
        "#windowSwitcher QPushButton:hover { background: #31496c; }"
        "#panelStatus { color: #b8c5d9; font-size: 12px; }"
        "#panelClock { color: #f3f6fb; font-size: 12px; font-weight: 600; min-width: 72px; }"
        "#startMenuPanel { background: #f4f7fb; border: 1px solid #8498ac; border-radius: 0px; }"
        "#startMenuPanel #pageTitle { font-size: 24px; }"
        "#startMenuPanel #applicationSearch { background: #f8faff; border: 1px solid #cfd9e8; border-radius: 8px; padding: 10px 12px; }"
        "#startMenuPanel #primaryAction { background: #3568d4; color: white; border: 0; border-radius: 6px; padding: 8px 14px; font-weight: 600; }");
    connect(settingsHub_, &SettingsHubWidget::themeChanged, this, &MainWindow::applyTheme);
    applyTheme(QSettings().value(QStringLiteral("appearance/theme"), QStringLiteral("neon")).toString());

    const bool waylandSession = !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")
        || qEnvironmentVariable("XDG_SESSION_TYPE").compare(QStringLiteral("wayland"), Qt::CaseInsensitive) == 0;
    wmctrlPath_ = QStandardPaths::findExecutable(QStringLiteral("wmctrl"));
    if (waylandSession) {
        windowSwitcherStatus_->setText(tr("Open windows are managed by your Wayland desktop."));
        windowSwitcherStatus_->setToolTip(tr("Global window listing and activation require a compositor-specific adapter."));
        workspaceButton_->setEnabled(false);
        workspaceButton_->setToolTip(tr("Virtual desktop controls are provided by your Wayland desktop."));
    } else if (wmctrlPath_.isEmpty()) {
        windowSwitcherStatus_->setText(tr("Install wmctrl to enable X11 app switching."));
        workspaceButton_->setEnabled(false);
        workspaceButton_->setToolTip(tr("Install wmctrl to enable virtual desktop controls in LinWin."));
    } else {
        windowSwitcherStatus_->hide();
        windowListProcess_ = new QProcess(this);
        connect(windowListProcess_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int code, QProcess::ExitStatus processStatus) {
            if (processStatus == QProcess::NormalExit && code == 0)
                updateExternalWindows(QString::fromLocal8Bit(windowListProcess_->readAllStandardOutput()));
            else if (windowSwitcherStatus_ != nullptr)
                windowSwitcherStatus_->setText(tr("Could not read the X11 window list."));
        });
        windowRefreshTimer_ = new QTimer(this);
        windowRefreshTimer_->setInterval(1500);
        connect(windowRefreshTimer_, &QTimer::timeout, this, &MainWindow::refreshExternalWindows);
        windowRefreshTimer_->start();
        refreshExternalWindows();

        auto *switchShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Tab), this);
        switchShortcut->setContext(Qt::ApplicationShortcut);
        connect(switchShortcut, &QShortcut::activated, this, &MainWindow::cycleExternalWindow);

        workspaceSupportAvailable_ = true;
        workspaceListProcess_ = new QProcess(this);
        connect(workspaceListProcess_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int code, QProcess::ExitStatus processStatus) {
            if (processStatus == QProcess::NormalExit && code == 0) {
                updateWorkspaces(QString::fromLocal8Bit(workspaceListProcess_->readAllStandardOutput()));
            } else if (workspaceButton_ != nullptr) {
                workspaceButton_->setEnabled(false);
                workspaceButton_->setToolTip(tr("The X11 window manager could not list virtual desktops."));
                if (workspaceOverflowAction_ != nullptr) workspaceOverflowAction_->setEnabled(false);
            }
        });
        workspaceCommandProcess_ = new QProcess(this);
        connect(workspaceCommandProcess_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int code, QProcess::ExitStatus processStatus) {
            if (processStatus != QProcess::NormalExit || code != 0) {
                if (workspaceButton_ != nullptr)
                    workspaceButton_->setToolTip(tr("The desktop change failed. Check that your X11 window manager supports virtual desktops."));
                return;
            }
            refreshWorkspaces();
        });
        workspaceRefreshTimer_ = new QTimer(this);
        workspaceRefreshTimer_->setInterval(2500);
        connect(workspaceRefreshTimer_, &QTimer::timeout, this, &MainWindow::refreshWorkspaces);
        workspaceRefreshTimer_->start();
        connect(workspaceMenu_, &QMenu::aboutToShow, this, &MainWindow::refreshWorkspaces);
        refreshWorkspaces();
    }

    updateShellGeometry();
}

void MainWindow::refreshExternalWindows()
{
    if (windowListProcess_ == nullptr || windowListProcess_->state() != QProcess::NotRunning) return;
    windowListProcess_->start(wmctrlPath_, {QStringLiteral("-lp")});
}

void MainWindow::updateExternalWindows(const QString &output)
{
    static const QRegularExpression rowPattern(QStringLiteral("^\\s*(0x[0-9a-fA-F]+)\\s+(-?\\d+)\\s+(\\d+)\\s+\\S+\\s+(.*)$"));
    QStringList currentOrder;
    const qint64 ownPid = QCoreApplication::applicationPid();
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const auto match = rowPattern.match(line);
        if (!match.hasMatch() || match.captured(3).toLongLong() == ownPid) continue;
        const QString id = match.captured(1);
        const QString title = match.captured(4).trimmed();
        if (title.isEmpty()) continue;
        currentOrder.append(id);
        QPushButton *button = windowButtons_.value(id, nullptr);
        if (button == nullptr) {
            button = new QPushButton(windowButtonsLayout_->parentWidget());
            button->setMaximumWidth(180);
            connect(button, &QPushButton::clicked, this, [this, id]() { activateExternalWindow(id); });
            windowButtons_.insert(id, button);
            windowButtonsLayout_->addWidget(button);
        }
        button->setText(title.size() > 20 ? title.left(19) + QChar(0x2026) : title);
        button->setToolTip(title);
    }

    for (auto it = windowButtons_.begin(); it != windowButtons_.end();) {
        if (!currentOrder.contains(it.key())) {
            windowButtonsLayout_->removeWidget(it.value());
            it.value()->deleteLater();
            it = windowButtons_.erase(it);
        } else {
            ++it;
        }
    }
    windowOrder_ = currentOrder;
    if (windowSwitcherStatus_ != nullptr) {
        windowSwitcherStatus_->setVisible(windowOrder_.isEmpty());
        if (windowOrder_.isEmpty()) windowSwitcherStatus_->setText(tr("No other open windows."));
    }
    if (windowCursor_ >= static_cast<int>(windowOrder_.size())) windowCursor_ = -1;
}

void MainWindow::activateExternalWindow(const QString &id)
{
    if (!QRegularExpression(QStringLiteral("^0x[0-9a-fA-F]+$")).match(id).hasMatch()) return;
    QProcess::startDetached(wmctrlPath_, {QStringLiteral("-ia"), id});
}

void MainWindow::showCommandPalette()
{
    if(commandPaletteOpen_ || quickControlsOpen_ || overviewOpen_) return;
    commandPaletteOpen_=true;
    using namespace LinWinVisual;
    QList<Command> commands;
    auto add=[&](const QString &id,const QString &title,const QString &keywords,Glyph glyph){
        commands.append({id,title,keywords,shellIcon(glyph)});
    };
    if(componentEnabled(QStringLiteral("files"))) add("files",tr("Open File Explorer"),tr("folders documents storage"),Glyph::Files);
    if(componentEnabled(QStringLiteral("task-manager"))) add("tasks",tr("Open Task Manager"),tr("cpu ram processes performance"),Glyph::Tasks);
    if(componentEnabled(QStringLiteral("clipboard"))) add("clipboard",tr("Open Clipboard history"),tr("copy paste"),Glyph::Clipboard);
    add("settings",tr("Open Settings"),tr("preferences configuration"),Glyph::Settings);
    add("overview",tr("Open Task View"),tr("switch windows overview"),Glyph::Desktop);
    add("desktop",tr("Show desktop / restore hidden tools"),tr("hide windows recall"),Glyph::Desktop);
    if(desktopSurface_) {
        add("quick",tr("Open Quick Controls"),tr("effects privacy graphics"),Glyph::Settings);
        auto *themes=settingsHub_->findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
        if(themes) for(int i=0;i<themes->count();++i)
            add(QStringLiteral("theme:")+themes->itemData(i).toString(),tr("Theme: %1").arg(themes->itemText(i)),tr("appearance personalization wallpaper"),Glyph::Settings);
    }
    if(!toolWindows_.isEmpty()) {
        QSettings settings;
        for(int slot=1;slot<=3;++slot) if(!LinWinWorkspace::load(settings,slot).isEmpty())
            add(QStringLiteral("workspace:%1").arg(slot),tr("Restore workspace %1").arg(slot),tr("saved layout windows"),Glyph::Snap);
    }
    CommandPalette palette(commands,this);
    const bool accepted=palette.exec()==QDialog::Accepted;
    const QString id=palette.chosen();
    commandPaletteOpen_=false;
    if(!accepted) return;
    if(id=="files") showPage(1);
    else if(id=="tasks") showPage(2);
    else if(id=="clipboard") showClipboardCenter();
    else if(id=="settings") showSettingsHub();
    else if(id=="overview") showWindowOverview();
    else if(id=="desktop") toggleWorkspaceVisibility();
    else if(id=="quick") showQuickControls();
    else if(id.startsWith("theme:")) {
        auto *themes=settingsHub_->findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
        if(themes) {
            const int index=themes->findData(id.mid(6));
            if(index>=0) themes->setCurrentIndex(index);
        }
    } else if(id.startsWith("workspace:")) {
        bool ok=false;const int slot=id.mid(10).toInt(&ok);
        if(ok && LinWinWorkspace::validSlot(slot)) loadWorkspaceProfile(slot);
    }
}

void MainWindow::showQuickControls()
{
    if(quickControlsOpen_ || !desktopSurface_) return;
    quickControlsOpen_=true;
    QDialog panel(this);
    panel.setObjectName(QStringLiteral("linwinQuickControls"));
    panel.setWindowTitle(tr("LinWin Quick Controls"));
    panel.resize(440,350);
    auto *layout=new QVBoxLayout(&panel);
    layout->setContentsMargins(22,18,22,18);
    layout->setSpacing(12);
    auto *title=new QLabel(tr("Quick controls"),&panel);
    title->setObjectName(QStringLiteral("pageTitle"));layout->addWidget(title);
    auto *intro=new QLabel(tr("Personalize the desktop. Changes apply immediately."),&panel);
    intro->setWordWrap(true);layout->addWidget(intro);
    auto *themeLabel=new QLabel(tr("Desktop theme"),&panel);
    auto *themes=new QComboBox(&panel);
    themes->setObjectName(QStringLiteral("linwinQuickTheme"));
    themeLabel->setBuddy(themes);layout->addWidget(themeLabel);layout->addWidget(themes);
    auto *settingsThemes=settingsHub_->findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
    if(settingsThemes) {
        for(int i=0;i<settingsThemes->count();++i) themes->addItem(settingsThemes->itemText(i),settingsThemes->itemData(i));
        themes->setCurrentIndex(settingsThemes->currentIndex());
    } else themes->setEnabled(false);
    QSettings preferences;
    auto *effects=new QCheckBox(tr("Desktop lighting and visual effects"),&panel);
    effects->setObjectName(QStringLiteral("linwinQuickEffects"));
    effects->setChecked(preferences.value(QStringLiteral("appearance/desktopEffects"),true).toBool());
    auto *gpu=new QCheckBox(tr("Prefer OpenGL rendering"),&panel);
    gpu->setObjectName(QStringLiteral("linwinQuickGpu"));
    gpu->setChecked(preferences.value(QStringLiteral("appearance/desktopGpu"),true).toBool());
    auto *previews=new QCheckBox(tr("Show window thumbnails in Task View"),&panel);
    previews->setObjectName(QStringLiteral("linwinQuickPreviews"));
    previews->setChecked(preferences.value(QStringLiteral("appearance/taskViewPreviews"),true).toBool());
    layout->addWidget(effects);layout->addWidget(gpu);layout->addWidget(previews);
    auto refreshControls=[themes,effects,gpu]{
        const bool minimal=themes->currentData().toString()==QStringLiteral("minimal");
        effects->setEnabled(!minimal);
        gpu->setEnabled(!minimal && effects->isChecked());
    };
    connect(themes,qOverload<int>(&QComboBox::currentIndexChanged),&panel,[settingsThemes,refreshControls](int index){
        if(settingsThemes) settingsThemes->setCurrentIndex(index);
        refreshControls();
    });
    connect(effects,&QCheckBox::toggled,&panel,[this,refreshControls](bool enabled){desktopSurface_->setEffectsEnabled(enabled);refreshControls();});
    connect(gpu,&QCheckBox::toggled,desktopSurface_,&DesktopSurface::setGpuPreferred);
    connect(previews,&QCheckBox::toggled,&panel,[](bool enabled){QSettings().setValue(QStringLiteral("appearance/taskViewPreviews"),enabled);});
    refreshControls();
    if(!toolWindows_.isEmpty()) {
        auto *profiles=new QComboBox(&panel);
        profiles->setObjectName(QStringLiteral("linwinQuickWorkspaceSlot"));
        for(int slot=1;slot<=3;++slot) profiles->addItem(tr("Workspace %1").arg(slot),slot);
        layout->addWidget(new QLabel(tr("Saved workspaces"),&panel));
        auto *row=new QHBoxLayout;
        auto *save=new QPushButton(tr("Save"),&panel);
        save->setObjectName(QStringLiteral("linwinQuickSaveWorkspace"));
        auto *restore=new QPushButton(tr("Restore"),&panel);
        restore->setObjectName(QStringLiteral("linwinQuickRestoreWorkspace"));
        row->addWidget(profiles,1);row->addWidget(save);row->addWidget(restore);layout->addLayout(row);
        connect(save,&QPushButton::clicked,&panel,[this,profiles]{saveWorkspaceProfile(profiles->currentData().toInt());});
        connect(restore,&QPushButton::clicked,&panel,[this,profiles,&panel]{const int slot=profiles->currentData().toInt();panel.accept();loadWorkspaceProfile(slot);});
    }
    layout->addStretch();
    auto *done=new QDialogButtonBox(QDialogButtonBox::Close,&panel);
    connect(done,&QDialogButtonBox::rejected,&panel,&QDialog::reject);
    layout->addWidget(done);
    panel.exec();
    quickControlsOpen_=false;
}

void MainWindow::showWindowOverview()
{
    if (overviewOpen_) return;
    overviewOpen_=true;
    QList<LinWinVisual::WindowCard> cards;
    for (int index=1;index<=4;++index) {
        auto *window=toolWindows_.value(index,nullptr);
        if (!window || !window->isEnabled() || (!window->isVisible() && !hiddenWorkspace_.contains(index))) continue;
        QIcon preview=window->windowIcon();
        const bool visible=window->isVisible() && !window->isMinimized();
        const bool capture=visible && index!=3 && QSettings().value(QStringLiteral("appearance/taskViewPreviews"),true).toBool();
        if (capture) {
            const QPixmap pixels=window->grab().scaled(192,108,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            if (!pixels.isNull()) preview=QIcon(pixels);
        }
        cards.append({window->windowTitle(),capture?tr("Native window preview"):(visible?tr("Window icon (privacy)"):tr("Hidden or minimized tool")),QStringLiteral("tool:%1").arg(index),preview});
    }
    for (const auto &id:windowOrder_) {
        auto *button=windowButtons_.value(id,nullptr);
        if (button) cards.append({button->toolTip(),tr("External X11 application"),id,LinWinVisual::shellIcon(LinWinVisual::Glyph::Desktop)});
        if(cards.size()>=48) break;
    }
    LinWinVisual::WindowOverview overview(cards,this);
    const int result=overview.exec();
    overviewOpen_=false;
    if(result!=QDialog::Accepted) return;
    const QString key=overview.chosenKey();
    if(key.startsWith(QStringLiteral("tool:"))) {
        bool ok=false;const int index=key.mid(5).toInt(&ok);
        if(ok && toolWindows_.contains(index)) showPage(index);
    } else if(windowOrder_.contains(key)) activateExternalWindow(key);
}

void MainWindow::cycleExternalWindow()
{
    if (windowOrder_.isEmpty()) return;
    windowCursor_ = (windowCursor_ + 1) % static_cast<int>(windowOrder_.size());
    activateExternalWindow(windowOrder_.at(windowCursor_));
}

void MainWindow::refreshWorkspaces()
{
    if (!workspaceSupportAvailable_ || workspaceListProcess_ == nullptr
        || workspaceListProcess_->state() != QProcess::NotRunning)
        return;
    workspaceListProcess_->start(wmctrlPath_, {QStringLiteral("-d")});
}

void MainWindow::updateWorkspaces(const QString &output)
{
    const QList<WorkspaceInfo> refreshed = parseWorkspaceList(output);
    if (refreshed.isEmpty()) {
        workspaceButton_->setEnabled(false);
        workspaceButton_->setText(tr("Desktops"));
        workspaceButton_->setToolTip(tr("No virtual desktops were reported by the X11 window manager."));
        if (workspaceOverflowAction_ != nullptr) workspaceOverflowAction_->setEnabled(false);
        return;
    }

    bool changed = refreshed.size() != workspaces_.size();
    if (!changed) {
        for (qsizetype i = 0; i < refreshed.size(); ++i) {
            if (refreshed.at(i).index != workspaces_.at(i).index
                || refreshed.at(i).name != workspaces_.at(i).name
                || refreshed.at(i).current != workspaces_.at(i).current) {
                changed = true;
                break;
            }
        }
    }
    workspaces_ = refreshed;
    workspaceButton_->setEnabled(true);
    if (workspaceOverflowAction_ != nullptr) workspaceOverflowAction_->setEnabled(true);
    const auto current = std::find_if(workspaces_.cbegin(), workspaces_.cend(),
                                     [](const WorkspaceInfo &workspace) { return workspace.current; });
    const WorkspaceInfo &selected = current != workspaces_.cend() ? *current : workspaces_.first();
    workspaceButton_->setText(selected.name + QStringLiteral("  ▾"));
    workspaceButton_->setToolTip(tr("Current virtual desktop: %1. Open the menu to switch or add desktops.")
                                     .arg(selected.name));
    if (changed) rebuildWorkspaceMenu();
}

void MainWindow::rebuildWorkspaceMenu()
{
    if (workspaceMenu_ == nullptr) return;
    workspaceMenu_->clear();
    for (const WorkspaceInfo &workspace : workspaces_) {
        auto *action = workspaceMenu_->addAction(workspace.name);
        action->setCheckable(true);
        action->setChecked(workspace.current);
        action->setEnabled(!workspace.current);
        connect(action, &QAction::triggered, this, [this, index = workspace.index]() {
            switchToWorkspace(index);
        });
    }
    if (!workspaces_.isEmpty()) workspaceMenu_->addSeparator();
    auto *addDesktop = workspaceMenu_->addAction(tr("＋  New desktop"));
    addDesktop->setEnabled(workspaces_.size() < 16);
    addDesktop->setToolTip(workspaces_.size() < 16
        ? tr("Add a virtual desktop in your X11 session")
        : tr("LinWin limits this menu to 16 virtual desktops"));
    connect(addDesktop, &QAction::triggered, this, &MainWindow::createWorkspace);
}

void MainWindow::switchToWorkspace(int index)
{
    if (!workspaceSupportAvailable_ || workspaceCommandProcess_ == nullptr
        || workspaceCommandProcess_->state() != QProcess::NotRunning)
        return;
    const bool exists = std::any_of(workspaces_.cbegin(), workspaces_.cend(),
                                    [index](const WorkspaceInfo &workspace) { return workspace.index == index; });
    if (!exists) return;
    workspaceCommandProcess_->start(wmctrlPath_, {QStringLiteral("-s"), QString::number(index)});
}

void MainWindow::createWorkspace()
{
    if (!workspaceSupportAvailable_ || workspaceCommandProcess_ == nullptr
        || workspaceCommandProcess_->state() != QProcess::NotRunning || workspaces_.isEmpty()
        || workspaces_.size() >= 16)
        return;
    workspaceCommandProcess_->start(wmctrlPath_,
                                    {QStringLiteral("-n"), QString::number(workspaces_.size() + 1)});
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateTaskbarLayout();
    updateShellGeometry();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type()==QEvent::WindowStateChange || event->type()==QEvent::Show || event->type()==QEvent::Hide) {
        for(auto *window:toolWindows_) if(watched==window) {
            QTimer::singleShot(0,this,[this]{syncToolButtons();});
            break;
        }
    }
    if (startMenu_ == nullptr || startButton_ == nullptr || !startMenu_->isVisible())
        return QMainWindow::eventFilter(watched, event);

    auto *widget = qobject_cast<QWidget *>(watched);
    if (widget == nullptr || (widget != this && !isAncestorOf(widget)))
        return QMainWindow::eventFilter(watched, event);

    const bool insideMenu = widget == startMenu_ || startMenu_->isAncestorOf(widget);
    const bool insideButton = widget == startButton_ || startButton_->isAncestorOf(widget);
    if (event->type() == QEvent::MouseButtonPress && !insideMenu && !insideButton) {
        // Let the original click continue to its intended control.
        startButton_->setChecked(false);
    } else if (event->type() == QEvent::KeyPress
               && static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape
               && (insideMenu || insideButton)) {
        startButton_->setChecked(false);
        startButton_->setFocus(Qt::ShortcutFocusReason);
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && startMenu_->isVisible()) {
        startMenu_->hide();
        startButton_->setChecked(false);
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::initializeToolWindows()
{
    const QList<QPair<QWidget *, QString>> tools{
        {fileManager_, tr("File Explorer")},
        {taskManager_, tr("Task Manager")},
        {clipboardHistory_, tr("Clipboard history")},
        {settingsHub_, tr("Settings")}
    };
    int index = 1;
    for (const auto &tool : tools) {
        pages_->removeWidget(tool.first);
        pages_->insertWidget(index, new QWidget(pages_));
        auto *window = new QDialog(this, Qt::Window | Qt::WindowTitleHint |
                                  Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint |
                                  Qt::WindowCloseButtonHint);
        window->setObjectName(QStringLiteral("linwinToolWindow_%1").arg(index));
        window->setWindowTitle(tool.second);
        const LinWinVisual::Glyph toolGlyphs[]{LinWinVisual::Glyph::Files,LinWinVisual::Glyph::Tasks,LinWinVisual::Glyph::Clipboard,LinWinVisual::Glyph::Settings};
        window->setWindowIcon(LinWinVisual::shellIcon(toolGlyphs[index-1]));
        window->setWindowModality(Qt::NonModal);
        window->setAttribute(Qt::WA_DeleteOnClose, false);
        auto *layout = new QVBoxLayout(window);
        layout->setContentsMargins(0,0,0,0);
        auto *snapBar=new QHBoxLayout;
        snapBar->setContentsMargins(10,6,10,6);
        auto *windowIdentity=new QLabel(tool.second,window);
        windowIdentity->setStyleSheet(QStringLiteral("font-weight:600; font-size:12px; color:#425773; background:transparent;"));
        snapBar->addWidget(windowIdentity);
        auto *snap=new SnapController(window,desktopArea_);
        snapBar->addStretch();
        snapBar->addWidget(snap->createButton());
        layout->addLayout(snapBar);
        layout->addWidget(tool.first);
        tool.first->show();
        window->resize(index == 3 ? 560 : 900, index == 3 ? 500 : 650);
        toolWindows_.insert(index, window);
        connect(window, &QDialog::finished, this, [this](int) { syncToolButtons(); });
        ++index;
    }
}

void MainWindow::hideToolWindows()
{
    for (auto *window : toolWindows_) window->hide();
    syncToolButtons();
}

void MainWindow::toggleWorkspaceVisibility()
{
    if (toolWindows_.isEmpty()) { showPage(0); return; }
    bool anyVisible=false;
    for (auto *window:toolWindows_) if (window->isVisible()) { anyVisible=true; break; }
    if (!anyVisible && !hiddenWorkspace_.isEmpty()) { restoreHiddenWorkspace(); return; }
    showPage(0);
}

void MainWindow::restoreHiddenWorkspace()
{
    const auto saved=hiddenWorkspace_;
    hiddenWorkspace_.clear();
    for (int index:saved) {
        auto *window=toolWindows_.value(index,nullptr);
        if (!window || !window->isEnabled()) continue;
        if (window->isMinimized()) window->showNormal(); else window->show();
        window->raise();
    }
    syncToolButtons();
}

void MainWindow::saveWorkspaceProfile(int slot)
{
    QList<LinWinWorkspace::Placement> placements;
    for(int index=1;index<=4;++index) {
        auto *window=toolWindows_.value(index,nullptr);
        if(!window || !window->isEnabled() || (!window->isVisible() && !hiddenWorkspace_.contains(index))) continue;
        const bool maximized=window->isMaximized();
        const QRect geometry=(maximized || window->isMinimized()) ? window->normalGeometry() : window->geometry();
        placements.append({index,geometry,maximized});
    }
    QSettings settings;
    if(placements.isEmpty()) { QMessageBox::information(this,tr("Workspace profiles"),tr("Open at least one LinWin tool before saving.")); return; }
    if(!LinWinWorkspace::load(settings,slot).isEmpty() &&
        QMessageBox::question(this,tr("Workspace profiles"),tr("Replace saved workspace %1?").arg(slot),
            QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    if(!LinWinWorkspace::save(settings,slot,placements))
        QMessageBox::warning(this,tr("Workspace profiles"),tr("The workspace could not be saved."));
}

void MainWindow::loadWorkspaceProfile(int slot)
{
    QSettings settings;
    const auto saved=LinWinWorkspace::load(settings,slot);
    if(saved.isEmpty()) return;
    for(const auto &placement:saved) {
        auto *window=toolWindows_.value(placement.tool,nullptr);
        if(!window || !window->isEnabled()) continue;
        QScreen *display=QGuiApplication::screenAt(placement.geometry.center());
        if(!display) display=screen();
        if(!display) continue;
        showPage(placement.tool);
        window->showNormal();
        const QRect target=LinWinWorkspace::fit(placement.geometry,display->availableGeometry());
        if(target.isValid()) window->setGeometry(target);
        if(placement.maximized) window->showMaximized();
    }
    syncToolButtons();
}

void MainWindow::initializeToolTaskbarActions()
{
    if(toolWindows_.isEmpty()) return;
    const QList<QPushButton *> buttons{filesButton_,tasksButton_,clipboardButton_,settingsButton_};
    for(int i=0;i<buttons.size();++i) {
        const int index=i+1;
        auto *button=buttons.at(i);
        auto *window=toolWindows_.value(index);
        button->setContextMenuPolicy(Qt::ActionsContextMenu);
        auto add=[button](const QString &label,const QString &id) {
            auto *action=new QAction(label,button);
            action->setObjectName(id);
            button->addAction(action);
            return action;
        };
        auto *open=add(tr("Open / restore"),QStringLiteral("linwinToolOpen"));
        auto *minimize=add(tr("Minimize"),QStringLiteral("linwinToolMinimize"));
        auto *maximize=add(tr("Maximize / restore"),QStringLiteral("linwinToolMaximize"));
        auto *close=add(tr("Close window"),QStringLiteral("linwinToolClose"));
        connect(open,&QAction::triggered,this,[this,index]{showPage(index);});
        connect(minimize,&QAction::triggered,this,[this,window]{
            if(window->isVisible() && window->isEnabled()) window->showMinimized();
            syncToolButtons();
        });
        connect(maximize,&QAction::triggered,this,[this,window,index]{
            if(!window->isEnabled()) return;
            if(!window->isVisible() || window->isMinimized()) showPage(index);
            if(window->isMaximized()) window->showNormal(); else window->showMaximized();
            syncToolButtons();
        });
        connect(close,&QAction::triggered,this,[this,window]{
            if(window->isVisible()) window->close();
            syncToolButtons();
        });
    }
    syncToolButtons();
}

void MainWindow::syncToolButtons()
{
    if (toolWindows_.isEmpty() || !homeButton_) return;
    const QList<QPushButton *> buttons{filesButton_, tasksButton_, clipboardButton_, settingsButton_};
    bool any = false;
    for (int i=0; i<buttons.size(); ++i) {
        auto *window=toolWindows_.value(i+1);
        const bool visible=window->isVisible();
        buttons.at(i)->setChecked(visible);
        for(auto *action:buttons.at(i)->actions()) {
            const auto id=action->objectName();
            if(id==QStringLiteral("linwinToolOpen") || id==QStringLiteral("linwinToolMaximize")) action->setEnabled(window->isEnabled());
            else if(id==QStringLiteral("linwinToolMinimize")) action->setEnabled(window->isEnabled() && visible && !window->isMinimized());
            else if(id==QStringLiteral("linwinToolClose")) action->setEnabled(visible);
        }
        any = any || visible;
    }
    homeButton_->setChecked(!any);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hiddenWorkspace_.clear();
    hideToolWindows();
    QMainWindow::closeEvent(event);
}

void MainWindow::showPage(int index)
{
    if ((index == 1 && !componentEnabled(QStringLiteral("files")))
        || (index == 2 && !componentEnabled(QStringLiteral("task-manager")))
        || (index == 3 && !componentEnabled(QStringLiteral("clipboard"))))
        return;
    startMenu_->hide();
    startButton_->setChecked(false);
    if (!toolWindows_.isEmpty()) {
        if (index == 0) {
            hiddenWorkspace_.clear();
            for (int i=1;i<=4;++i) {
                auto *window=toolWindows_.value(i,nullptr);
                if (window && window->isVisible() && !window->isMinimized()) hiddenWorkspace_.append(i);
            }
            hideToolWindows();
            return;
        }
        hiddenWorkspace_.removeAll(index);
        auto *window = toolWindows_.value(index, nullptr);
        if (!window) return;
        if (!window->isVisible() && !window->property("linwinOpenedOnce").toBool()) {
            const QRect area = screen() ? screen()->availableGeometry() : geometry();
            const QSize size = window->size().boundedTo(area.size());
            window->resize(size);
            window->move(area.center() - QPoint(size.width()/2, size.height()/2) + QPoint((index-1)*20,(index-1)*16));
        }
        window->setProperty("linwinOpenedOnce",true);
        if (window->isMinimized()) window->showNormal(); else window->show();
        window->raise();
        window->activateWindow();
        syncToolButtons();
        return;
    }
    pages_->setCurrentIndex(index);
}

void MainWindow::showClipboardCenter()
{
    if (!componentEnabled(QStringLiteral("clipboard"))) return;
    showPage(3);
    if (clipboardHistory_ != nullptr) clipboardHistory_->focusSearch();
}

void MainWindow::showClipboardQuickPicker()
{
    if (!componentEnabled(QStringLiteral("clipboard")) || clipboardHistory_ == nullptr) return;
    clipboardHistory_->showQuickPicker(this);
}

void MainWindow::showSettingsHub()
{
    showPage(4);
    if (settingsHub_ != nullptr) settingsHub_->focusSearch();
}

void MainWindow::applyTheme(const QString &id)
{
    const auto &theme = LinWinVisual::theme(id);
    themeId_ = LinWinVisual::qstr(theme.id);
    QSettings().setValue(QStringLiteral("appearance/theme"), themeId_);
    if (desktopSurface_) desktopSurface_->setTheme(themeId_);
    if (desktopWidgets_) desktopWidgets_->setVisible(theme.widgets && width() >= 1100 && height() >= 640);
    applyTextScale(settingsHub_->textScalePercent());
}

void MainWindow::applyTextScale(int percent)
{
    if (percent < 85 || percent > 150 || baseStyleSheet_.isEmpty()) return;
    setStyleSheet(linwinScaledStyleSheet(baseStyleSheet_ + LinWinVisual::shellStyle(LinWinVisual::theme(themeId_)), percent));
}

bool MainWindow::componentEnabled(const QString &component) const
{
    QSettings settings;
    return settings.value(QStringLiteral("components/%1").arg(component), true).toBool();
}

void MainWindow::updateTaskbarLayout()
{
    const bool compact = width() < 920;
    const bool hideWindowDetails = width() < 920;

    if (startButton_ != nullptr) startButton_->setVisible(startMenuEnabled_);
    if (homeButton_ != nullptr) homeButton_->setVisible(!compact);
    if (filesButton_ != nullptr) filesButton_->setVisible(filesEnabled_);
    if (tasksButton_ != nullptr) tasksButton_->setVisible(taskManagerEnabled_);
    if (clipboardButton_ != nullptr) clipboardButton_->setVisible(clipboardEnabled_ && !compact);
    if (settingsButton_ != nullptr) settingsButton_->setVisible(!compact);
    if (signOutButton_ != nullptr) signOutButton_->setVisible(!compact);
    if (overflowButton_ != nullptr) overflowButton_->setVisible(compact);
    if (workspaceButton_ != nullptr) workspaceButton_->setVisible(!compact);

    if (clipboardOverflowAction_ != nullptr)
        clipboardOverflowAction_->setVisible(compact && clipboardEnabled_);
    if (settingsOverflowAction_ != nullptr)
        settingsOverflowAction_->setVisible(compact);
    if (desktopOverflowAction_ != nullptr)
        desktopOverflowAction_->setVisible(compact);
    if (signOutOverflowAction_ != nullptr)
        signOutOverflowAction_->setVisible(compact);
    if (workspaceOverflowAction_ != nullptr)
        workspaceOverflowAction_->setVisible(compact && workspaceSupportAvailable_);

    if (windowSwitcher_ != nullptr) windowSwitcher_->setVisible(!hideWindowDetails);
    if (systemStatus_ != nullptr) systemStatus_->setVisible(!hideWindowDetails);
}

void MainWindow::returnToLinuxDesktop()
{
    if (shellSessionMode_) return;
    hiddenWorkspace_.clear();
    hideToolWindows();
    showMinimized();
}

void MainWindow::applyComponentVisibility(const QString &component, bool enabled)
{
    if (component == QStringLiteral("start-menu")) {
        startMenuEnabled_ = enabled;
        if (!enabled) {
            startMenu_->hide();
            startButton_->setChecked(false);
        }
        updateTaskbarLayout();
        return;
    }

    int pageIndex = -1;
    QPushButton *button = nullptr;
    if (component == QStringLiteral("files")) { pageIndex = 1; button = filesButton_; }
    else if (component == QStringLiteral("task-manager")) { pageIndex = 2; button = tasksButton_; }
    else if (component == QStringLiteral("clipboard")) { pageIndex = 3; button = clipboardButton_; }
    if (pageIndex < 0 || button == nullptr) return;

    if (component == QStringLiteral("files")) filesEnabled_ = enabled;
    else if (component == QStringLiteral("task-manager")) taskManagerEnabled_ = enabled;
    else if (component == QStringLiteral("clipboard")) clipboardEnabled_ = enabled;
    if (auto *window = toolWindows_.value(pageIndex, nullptr)) {
        window->setEnabled(enabled);
        if (!enabled) { window->hide(); hiddenWorkspace_.removeAll(pageIndex); }
        syncToolButtons();
    } else pages_->widget(pageIndex)->setEnabled(enabled);
    if (component == QStringLiteral("clipboard")) clipboardHistory_->setComponentEnabled(enabled);
    if (!enabled && pages_->currentIndex() == pageIndex) showPage(0);
    updateTaskbarLayout();
}

void MainWindow::updateShellGeometry()
{
    if (desktopArea_ == nullptr || pages_ == nullptr || startMenu_ == nullptr) {
        return;
    }

    pages_->setGeometry(desktopArea_->rect());
    if (desktopWidgets_) desktopWidgets_->setVisible(LinWinVisual::theme(themeId_).widgets && width() >= 1100 && height() >= 640);
    const int areaWidth = desktopArea_->width();
    const int areaHeight = desktopArea_->height();
    const int menuWidth = qMax(1, qMin(560, areaWidth));
    const int menuHeight = qMax(1, qMin(620, areaHeight));
    startMenu_->setGeometry(0, qMax(0, areaHeight - menuHeight), menuWidth, menuHeight);
}
