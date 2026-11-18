// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "setup_wizard.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("LinWin"));
    QCoreApplication::setOrganizationName(QStringLiteral("LinWin contributors"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("LinWin Linux desktop shell"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption setupOption(QStringLiteral("setup"), QStringLiteral("Open the graphical LinWin setup wizard."));
    QCommandLineOption uninstallOption(QStringLiteral("uninstall"), QStringLiteral("Open the graphical LinWin uninstall wizard."));
    QCommandLineOption shellOption(QStringLiteral("shell"), QStringLiteral("Run LinWin as the full-screen desktop shell."));
    parser.addOption(setupOption);
    parser.addOption(uninstallOption);
    parser.addOption(shellOption);
    parser.process(app);

    if (parser.isSet(setupOption) || parser.isSet(uninstallOption)) {
        SetupWizard wizard(parser.isSet(uninstallOption));
        wizard.show();
        return app.exec();
    }

    MainWindow window(parser.isSet(shellOption));
    if (parser.isSet(shellOption))
        window.showFullScreen();
    else
        window.show();
    return app.exec();
}
