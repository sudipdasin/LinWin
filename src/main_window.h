// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QList>
#include <QMainWindow>
#include <QString>
#include <QStringList>

#include "workspace_state.h"

class AppLauncherWidget;
class DesktopSurface;
class DesktopWidgets;
class ClipboardHistoryWidget;
class FileManagerWidget;
class TaskManagerWidget;
class SettingsHubWidget;
class QAction;

class QHBoxLayout;
class QDialog;
class QCloseEvent;
class QLabel;
class QKeyEvent;
class QMenu;
class QProcess;
class QScrollArea;
class QTimer;
class QPushButton;
class QResizeEvent;
class QStackedWidget;
class QWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(bool overlayMode = true, bool shellSessionMode = false, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void showPage(int index);
    void initializeToolWindows();
    void rememberToolGeometry(int index);
    void hideToolWindows();
    void toggleWorkspaceVisibility();
    void restoreHiddenWorkspace();
    void saveWorkspaceProfile(int slot);
    void loadWorkspaceProfile(int slot);
    void syncToolButtons();
    void initializeToolTaskbarActions();
    void updateShellGeometry();
    void refreshExternalWindows();
    void updateActiveWindow(const QString &output);
    void updateExternalWindows(const QString &output);
    void activateExternalWindow(const QString &id);
    void cycleExternalWindow();
    void showWindowOverview();
    void showQuickControls();
    void showCommandPalette();
    bool commandPaletteOpen_=false;
    bool quickControlsOpen_ = false;
    bool overviewOpen_ = false;
    void refreshWorkspaces();
    void updateWorkspaces(const QString &output);
    void rebuildWorkspaceMenu();
    void switchToWorkspace(int index);
    void createWorkspace();
    void showClipboardCenter();
    void showClipboardQuickPicker();
    void showSettingsHub();
    void applyTextScale(int percent);
    void applyTheme(const QString &id);
    void updateTaskbarLayout();
    void applyComponentVisibility(const QString &component, bool enabled);
    void returnToLinuxDesktop();
    void requestCloseLinWin();
    bool componentEnabled(const QString &component) const;

    QHash<int, QDialog *> toolWindows_;
    QList<int> hiddenWorkspace_;
    QWidget *desktopArea_ = nullptr;
    DesktopSurface *desktopSurface_ = nullptr;
    DesktopWidgets *desktopWidgets_ = nullptr;
    QString themeId_ = QStringLiteral("neon");
    QStackedWidget *pages_ = nullptr;
    AppLauncherWidget *startMenu_ = nullptr;
    QPushButton *startButton_ = nullptr;
    QPushButton *homeButton_ = nullptr;
    QPushButton *filesButton_ = nullptr;
    QPushButton *tasksButton_ = nullptr;
    QPushButton *clipboardButton_ = nullptr;
    QPushButton *settingsButton_ = nullptr;
    QPushButton *signOutButton_ = nullptr;
    QPushButton *closeButton_ = nullptr;
    QPushButton *overflowButton_ = nullptr;
    QPushButton *workspaceButton_ = nullptr;
    QAction *clipboardOverflowAction_ = nullptr;
    QAction *settingsOverflowAction_ = nullptr;
    QAction *desktopOverflowAction_ = nullptr;
    QAction *signOutOverflowAction_ = nullptr;
    QAction *returnDesktopOverflowAction_ = nullptr;
    QAction *closeOverflowAction_ = nullptr;
    QAction *workspaceOverflowAction_ = nullptr;
    QMenu *workspaceMenu_ = nullptr;
    QScrollArea *windowSwitcher_ = nullptr;
    QLabel *systemStatus_ = nullptr;
    QLabel *clock_ = nullptr;
    FileManagerWidget *fileManager_ = nullptr;
    TaskManagerWidget *taskManager_ = nullptr;
    ClipboardHistoryWidget *clipboardHistory_ = nullptr;
    SettingsHubWidget *settingsHub_ = nullptr;
    QHBoxLayout *windowButtonsLayout_ = nullptr;
    QLabel *windowSwitcherStatus_ = nullptr;
    QProcess *windowListProcess_ = nullptr,*activeWindowProcess_=nullptr;
    QString xpropPath_;
    QTimer *windowRefreshTimer_ = nullptr;
    QProcess *workspaceListProcess_ = nullptr;
    QProcess *workspaceCommandProcess_ = nullptr;
    QTimer *workspaceRefreshTimer_ = nullptr;
    QHash<QString, QPushButton *> windowButtons_;
    QStringList windowOrder_;
    QList<WorkspaceInfo> workspaces_;
    QString wmctrlPath_;
    int windowCursor_ = -1;
    bool startMenuEnabled_ = true;
    bool filesEnabled_ = true;
    bool taskManagerEnabled_ = true;
    bool clipboardEnabled_ = true;
    bool workspaceSupportAvailable_ = false;
    bool overlayMode_ = true;
    bool shellSessionMode_ = false;
    QString baseStyleSheet_;
};
