// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class AppLauncherWidget;
class ClipboardHistoryWidget;
class FileManagerWidget;
class TaskManagerWidget;
class SettingsHubWidget;
class QAction;

class QHBoxLayout;
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
    explicit MainWindow(bool shellMode = false, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void showPage(int index);
    void updateShellGeometry();
    void refreshExternalWindows();
    void updateExternalWindows(const QString &output);
    void activateExternalWindow(const QString &id);
    void cycleExternalWindow();
    void showClipboardCenter();
    void showClipboardQuickPicker();
    void showSettingsHub();
    void applyTextScale(int percent);
    void updateTaskbarLayout();
    void applyComponentVisibility(const QString &component, bool enabled);
    bool componentEnabled(const QString &component) const;

    QWidget *desktopArea_ = nullptr;
    QStackedWidget *pages_ = nullptr;
    AppLauncherWidget *startMenu_ = nullptr;
    QPushButton *startButton_ = nullptr;
    QPushButton *homeButton_ = nullptr;
    QPushButton *filesButton_ = nullptr;
    QPushButton *tasksButton_ = nullptr;
    QPushButton *clipboardButton_ = nullptr;
    QPushButton *settingsButton_ = nullptr;
    QPushButton *signOutButton_ = nullptr;
    QPushButton *overflowButton_ = nullptr;
    QAction *clipboardOverflowAction_ = nullptr;
    QAction *settingsOverflowAction_ = nullptr;
    QAction *signOutOverflowAction_ = nullptr;
    QScrollArea *windowSwitcher_ = nullptr;
    QLabel *systemStatus_ = nullptr;
    QLabel *clock_ = nullptr;
    FileManagerWidget *fileManager_ = nullptr;
    TaskManagerWidget *taskManager_ = nullptr;
    ClipboardHistoryWidget *clipboardHistory_ = nullptr;
    SettingsHubWidget *settingsHub_ = nullptr;
    QHBoxLayout *windowButtonsLayout_ = nullptr;
    QLabel *windowSwitcherStatus_ = nullptr;
    QProcess *windowListProcess_ = nullptr;
    QTimer *windowRefreshTimer_ = nullptr;
    QHash<QString, QPushButton *> windowButtons_;
    QStringList windowOrder_;
    QString wmctrlPath_;
    int windowCursor_ = -1;
    bool startMenuEnabled_ = true;
    bool filesEnabled_ = true;
    bool taskManagerEnabled_ = true;
    bool clipboardEnabled_ = true;
    QString baseStyleSheet_;
};
