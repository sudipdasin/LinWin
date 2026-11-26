// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "setup_wizard.h"
#include "native_benchmark.h"

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
    QCommandLineOption shellOption(QStringLiteral("shell"), QStringLiteral("Run LinWin as the selectable X11 desktop session."));
    QCommandLineOption windowOption(QStringLiteral("window"), QStringLiteral("Open LinWin in a normal window instead of covering the desktop."));
    parser.addOption(setupOption);
    parser.addOption(uninstallOption);
    parser.addOption(shellOption);
    parser.addOption(windowOption);
    const QCommandLineOption benchmarkOption(QStringLiteral("benchmark-native"),QStringLiteral("Run isolated native offscreen benchmark."),QStringLiteral("theme"));
    parser.addOption(benchmarkOption);
    parser.process(app);
    if(parser.isSet(benchmarkOption)) return LinWinBench::run(app,parser.value(benchmarkOption));

    if (parser.isSet(setupOption) || parser.isSet(uninstallOption)) {
        SetupWizard wizard(parser.isSet(uninstallOption));
        wizard.show();
        return app.exec();
    }

    const bool shellSession = parser.isSet(shellOption);
    const bool overlayMode = shellSession || !parser.isSet(windowOption);
    MainWindow window(overlayMode, shellSession);
    if (overlayMode)
        window.showFullScreen();
    else
        window.show();
    return app.exec();
}
