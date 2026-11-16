// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QMainWindow>

class AppLauncherWidget;
class FileManagerWidget;
class QKeyEvent;
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

    QWidget *desktopArea_ = nullptr;
    QStackedWidget *pages_ = nullptr;
    AppLauncherWidget *startMenu_ = nullptr;
    QPushButton *startButton_ = nullptr;
    QPushButton *homeButton_ = nullptr;
    QPushButton *filesButton_ = nullptr;
    QPushButton *clipboardButton_ = nullptr;
    FileManagerWidget *fileManager_ = nullptr;
};
