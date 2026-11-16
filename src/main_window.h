// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QMainWindow>

class QTabWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QTabWidget *pages_ = nullptr;
};
