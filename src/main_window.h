// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class AppLauncherWidget;
class FileManagerWidget;
class TaskManagerWidget;

class QHBoxLayout;
class QLabel;
class QKeyEvent;
class QProcess;
class QTimer;
class QPushButton;
class QResizeEvent;
class QStackedWidget;
class QWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

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

    QWidget *desktopArea_ = nullptr;
    QStackedWidget *pages_ = nullptr;
    AppLauncherWidget *startMenu_ = nullptr;
    QPushButton *startButton_ = nullptr;
    QPushButton *homeButton_ = nullptr;
    QPushButton *filesButton_ = nullptr;
    QPushButton *tasksButton_ = nullptr;
    QPushButton *clipboardButton_ = nullptr;
    FileManagerWidget *fileManager_ = nullptr;
    TaskManagerWidget *taskManager_ = nullptr;
    QHBoxLayout *windowButtonsLayout_ = nullptr;
    QLabel *windowSwitcherStatus_ = nullptr;
    QProcess *windowListProcess_ = nullptr;
    QTimer *windowRefreshTimer_ = nullptr;
    QHash<QString, QPushButton *> windowButtons_;
    QStringList windowOrder_;
    QString wmctrlPath_;
    int windowCursor_ = -1;
};
