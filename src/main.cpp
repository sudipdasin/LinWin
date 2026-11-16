// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("LinWin"));
    QCoreApplication::setOrganizationName(QStringLiteral("LinWin contributors"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    MainWindow window;
    window.show();
    return app.exec();
}
