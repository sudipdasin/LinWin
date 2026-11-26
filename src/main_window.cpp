// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"

#include "app_launcher_widget.h"
#include "clipboard_history_widget.h"
#include "file_manager_widget.h"
#include "system_profile.h"
#include "settings_hub_widget.h"
#include "task_manager_widget.h"
#include "text_scale.h"

#include <QDateTime>
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

QPushButton *makeTaskbarButton(const QString &label, const QString &name,
                               QStyle::StandardPixmap icon, QWidget *parent)
{
    auto *button = makePanelButton(QString(), name, parent);
    button->setIcon(parent->style()->standardIcon(icon));
    button->setIconSize(QSize(22, 22));
    button->setFixedWidth(42);
    button->setAccessibleName(label);
    button->setToolTip(label);
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
        auto *desktop = new QWidget(pages_);
        desktop->setObjectName(QStringLiteral("windowsDesktop"));
        auto *desktopLayout = new QVBoxLayout(desktop);
        desktopLayout->setContentsMargins(20, 20, 20, 20);
        desktopLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        auto *shortcutColumn = new QVBoxLayout;
        shortcutColumn->setSpacing(8);
        const auto addDesktopShortcut = [this, desktop, shortcutColumn](const QString &text,
                                                                        QStyle::StandardPixmap icon,
                                                                        int pageIndex) {
            auto *shortcut = new QToolButton(desktop);
            shortcut->setObjectName(QStringLiteral("desktopShortcut"));
            shortcut->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            shortcut->setIcon(style()->standardIcon(icon));
            shortcut->setIconSize(QSize(40, 40));
            shortcut->setText(text);
            shortcut->setFixedSize(92, 82);
            shortcut->setFocusPolicy(Qt::StrongFocus);
            connect(shortcut, &QToolButton::clicked, this, [this, pageIndex]() { showPage(pageIndex); });
            shortcutColumn->addWidget(shortcut, 0, Qt::AlignLeft);
        };
        addDesktopShortcut(tr("Files"), QStyle::SP_DirHomeIcon, 1);
        addDesktopShortcut(tr("Task Manager"), QStyle::SP_ComputerIcon, 2);
        addDesktopShortcut(tr("Settings"), QStyle::SP_FileDialogDetailedView, 4);
        desktopLayout->addLayout(shortcutColumn);
        desktopLayout->addStretch();
        desktop->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(desktop, &QWidget::customContextMenuRequested, this,
                [this, desktop](const QPoint &position) {
            QMenu menu(desktop);
            menu.addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), tr("Open Files"),
                           this, [this]() { showPage(1); });
            menu.addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Show apps"),
                           this, [this]() {
                if (startMenuEnabled_) startButton_->setChecked(true);
            });
            menu.addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("Personalize"),
                           this, &MainWindow::showSettingsHub);
            if (overlayMode_ && !shellSessionMode_) {
                menu.addSeparator();
                menu.addAction(tr("Switch to Linux desktop"), this, &MainWindow::returnToLinuxDesktop);
                menu.addAction(tr("Close LinWin"), this, &QWidget::close);
            }
            menu.exec(desktop->mapToGlobal(position));
        });
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
    connect(settingsHub_, &SettingsHubWidget::componentEnabledChanged,
            this, &MainWindow::applyComponentVisibility);
    connect(settingsHub_, &SettingsHubWidget::textScaleChanged,
            this, &MainWindow::applyTextScale);

    startMenu_ = new AppLauncherWidget(desktopArea_);
    startMenu_->setObjectName(QStringLiteral("startMenuPanel"));
    startMenu_->setWindowFlags(Qt::Widget);
    startMenu_->hide();

    auto *taskbar = new QFrame(root);
    taskbar->setObjectName(QStringLiteral("taskbar"));
    taskbar->setFixedHeight(62);
    auto *taskbarLayout = new QHBoxLayout(taskbar);
    taskbarLayout->setContentsMargins(12, 7, 12, 7);
    taskbarLayout->setSpacing(8);

    startButton_ = makePanelButton(QStringLiteral("⊞"), QStringLiteral("startButton"), taskbar);
    startButton_->setFixedWidth(42);
    startButton_->setAccessibleName(tr("Start"));
    startButton_->setToolTip(tr("Start — search and launch applications (Super or Ctrl+Esc)"));
    homeButton_ = makeTaskbarButton(tr("Desktop"), QStringLiteral("desktopButton"),
                                    QStyle::SP_DesktopIcon, taskbar);
    filesButton_ = makeTaskbarButton(tr("Files"), QStringLiteral("filesButton"),
                                     QStyle::SP_DirIcon, taskbar);
    tasksButton_ = makeTaskbarButton(tr("Task Manager"), QStringLiteral("tasksButton"),
                                     QStyle::SP_ComputerIcon, taskbar);
    tasksButton_->setToolTip(tr("Open Task Manager (Ctrl+Shift+Esc)"));
    clipboardButton_ = makeTaskbarButton(tr("Clipboard"), QStringLiteral("clipboardButton"),
                                         QStyle::SP_FileDialogListView, taskbar);
    settingsButton_ = makeTaskbarButton(tr("Settings"), QStringLiteral("settingsButton"),
                                        QStyle::SP_FileDialogDetailedView, taskbar);
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
    windowScroll->setFixedHeight(44);
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
    updateTaskbarLayout();

    connect(startButton_, &QPushButton::toggled, this, [this](bool visible) {
        startMenu_->setVisible(visible);
        if (visible) {
            updateShellGeometry();
            startMenu_->raise();
            startMenu_->focusSearch();
        }
    });
    connect(homeButton_, &QPushButton::clicked, this, [this]() { showPage(0); });
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
        "#windowsDesktop { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #06316f, stop:0.38 #0876bd, stop:0.7 #20a2d7, stop:1 #c6efff); }"
        "#desktopShortcut { background: transparent; color: white; border: 1px solid transparent; border-radius: 5px; padding: 4px; }"
        "#desktopShortcut:hover { background: rgba(255,255,255,28); border-color: rgba(255,255,255,65); }"
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
        "#taskbar { background: rgba(18, 26, 43, 242); border-top: 1px solid rgba(255,255,255,28); }"
        "#taskbar QPushButton { background: transparent; color: #e5ecf7; border: 0; border-radius: 7px; padding: 7px; font-size: 13px; }"
        "#taskbar QPushButton:hover { background: #31435f; }"
        "#taskbar QPushButton:checked { background: #304d76; color: white; border-bottom: 2px solid #68a5ff; }"
        "#taskbar #startButton { background: #253d67; color: white; font-size: 22px; font-weight: 700; padding: 0; }"
        "#taskbar #startButton:hover { background: #365987; }"
        "#windowSwitcherStatus { color: #9dabc2; font-size: 11px; padding: 0 8px; }"
        "#windowSwitcher QPushButton { background: #243550; color: #e5ecf7; border: 1px solid #354966; border-radius: 6px; padding: 6px 9px; }"
        "#windowSwitcher QPushButton:hover { background: #31496c; }"
        "#panelStatus { color: #b8c5d9; font-size: 12px; }"
        "#panelClock { color: #f3f6fb; font-size: 12px; font-weight: 600; min-width: 72px; }"
        "#startMenuPanel { background: #f3f6fc; border: 1px solid #d8e0ec; border-radius: 14px; }"
        "#startMenuPanel #pageTitle { font-size: 24px; }"
        "#startMenuPanel #applicationSearch { background: #f8faff; border: 1px solid #cfd9e8; border-radius: 8px; padding: 10px 12px; }"
        "#startMenuPanel #primaryAction { background: #3568d4; color: white; border: 0; border-radius: 6px; padding: 8px 14px; font-weight: 600; }");
    applyTextScale(settingsHub_->textScalePercent());

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

void MainWindow::showPage(int index)
{
    if ((index == 1 && !componentEnabled(QStringLiteral("files")))
        || (index == 2 && !componentEnabled(QStringLiteral("task-manager")))
        || (index == 3 && !componentEnabled(QStringLiteral("clipboard"))))
        return;
    pages_->setCurrentIndex(index);
    startMenu_->hide();
    startButton_->setChecked(false);
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

void MainWindow::applyTextScale(int percent)
{
    if (percent < 85 || percent > 150 || baseStyleSheet_.isEmpty()) return;
    setStyleSheet(linwinScaledStyleSheet(baseStyleSheet_, percent));
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
    pages_->widget(pageIndex)->setEnabled(enabled);
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
    const int areaWidth = desktopArea_->width();
    const int areaHeight = desktopArea_->height();
    const int menuWidth = qMin(520, qMax(300, areaWidth - 24));
    const int menuHeight = qMin(610, qMax(260, areaHeight - 24));
    startMenu_->setGeometry(qMax(12, (areaWidth - menuWidth) / 2),
                             qMax(12, areaHeight - menuHeight - 12), menuWidth, menuHeight);
}
