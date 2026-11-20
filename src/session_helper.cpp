// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <cstdio>
#include "xfce_dependency_plan.h"

#include <unistd.h>

namespace {
const QString kSessionFile = QStringLiteral("/usr/share/xsessions/linwin.desktop");
const QByteArray kSessionContents =
    "[Desktop Entry]\n"
    "Name=LinWin\n"
    "Comment=LinWin desktop shell (X11)\n"
    "Exec=/usr/libexec/linwin-session\n"
    "TryExec=/usr/libexec/linwin-session\n"
    "Type=Application\n"
    "DesktopNames=LinWin\n"
    "X-LinWin-Session=true\n";

bool installSession(QString *error)
{
    if (!QDir().mkpath(QStringLiteral("/usr/share/xsessions"))) {
        *error = QStringLiteral("Could not create /usr/share/xsessions.");
        return false;
    }
    QFile existing(kSessionFile);
    if (existing.exists()) {
        if (!existing.open(QIODevice::ReadOnly) || existing.readAll() != kSessionContents) {
            *error = QStringLiteral("A different session already uses /usr/share/xsessions/linwin.desktop. It was left untouched.");
            return false;
        }
        return true;
    }
    QSaveFile output(kSessionFile);
    if (!output.open(QIODevice::WriteOnly) || output.write(kSessionContents) != kSessionContents.size() || !output.commit()) {
        *error = QStringLiteral("Could not write the LinWin login-session entry: %1").arg(output.errorString());
        return false;
    }
    QFile::setPermissions(kSessionFile, QFileDevice::ReadOwner | QFileDevice::WriteOwner
        | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    return true;
}

bool removeSession(QString *error)
{
    QFile file(kSessionFile);
    if (!file.exists())
        return true;
    if (!file.open(QIODevice::ReadOnly) || file.readAll() != kSessionContents) {
        *error = QStringLiteral("The session entry at /usr/share/xsessions/linwin.desktop is not recognized as LinWin; it was left untouched.");
        return false;
    }
    if (!file.remove()) {
        *error = QStringLiteral("Could not remove the LinWin login-session entry: %1").arg(file.errorString());
        return false;
    }
    return true;
}

bool installXfceDependencies(QString *error)
{
    QFile osRelease(QStringLiteral("/etc/os-release"));
    if (!osRelease.open(QIODevice::ReadOnly)) {
        *error = QStringLiteral("Could not detect this distribution from /etc/os-release.");
        return false;
    }
    const PackageManager manager = packageManagerForOsRelease(QString::fromUtf8(osRelease.readAll()));
    const QString managerProgram = packageManagerProgram(manager);
    const QString program = managerProgram.isEmpty() ? QString() : QStandardPaths::findExecutable(managerProgram);
    const QStringList packages = xfceDependencyPackages(manager);
    const QStringList arguments = xfceDependencyInstallArguments(manager);
    if (program.isEmpty() || packages.isEmpty() || arguments.isEmpty()) {
        *error = QStringLiteral("Automatic XFCE dependency installation is not supported on this distribution. Use its graphical software manager to install: xfwm4, xfce4-settings, and the XFCE desktop manager package.");
        return false;
    }

    QProcess packageProcess;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    if (manager == PackageManager::Apt)
        environment.insert(QStringLiteral("DEBIAN_FRONTEND"), QStringLiteral("noninteractive"));
    packageProcess.setProcessEnvironment(environment);
    packageProcess.setProcessChannelMode(QProcess::SeparateChannels);
    packageProcess.start(program, arguments);
    if (!packageProcess.waitForStarted(10000)) {
        *error = QStringLiteral("Could not start %1: %2").arg(QFileInfo(program).fileName(), packageProcess.errorString());
        return false;
    }

    while (packageProcess.state() != QProcess::NotRunning) {
        packageProcess.waitForReadyRead(250);
        const QByteArray output = packageProcess.readAllStandardOutput();
        const QByteArray diagnostic = packageProcess.readAllStandardError();
        if (!output.isEmpty()) { std::fwrite(output.constData(), 1, static_cast<size_t>(output.size()), stdout); std::fflush(stdout); }
        if (!diagnostic.isEmpty()) { std::fwrite(diagnostic.constData(), 1, static_cast<size_t>(diagnostic.size()), stderr); std::fflush(stderr); }
    }
    const QByteArray output = packageProcess.readAllStandardOutput();
    const QByteArray diagnostic = packageProcess.readAllStandardError();
    if (!output.isEmpty()) { std::fwrite(output.constData(), 1, static_cast<size_t>(output.size()), stdout); std::fflush(stdout); }
    if (!diagnostic.isEmpty()) { std::fwrite(diagnostic.constData(), 1, static_cast<size_t>(diagnostic.size()), stderr); std::fflush(stderr); }
    if (packageProcess.exitStatus() != QProcess::NormalExit || packageProcess.exitCode() != 0) {
        *error = QStringLiteral("The distribution package manager could not install the XFCE session components (exit code %1).").arg(packageProcess.exitCode());
        return false;
    }
    return true;
}

QStringList removePackageCommand(QString *program, QString *error)
{
    const QString id = QFileInfo(QStringLiteral("/etc/os-release")).exists()
        ? QStringLiteral("/etc/os-release") : QString();
    QFile osRelease(id);
    QString content;
    if (osRelease.open(QIODevice::ReadOnly))
        content = QString::fromUtf8(osRelease.readAll()).toLower();
    content.remove(QLatin1Char('"'));
    content.remove(QLatin1Char('\''));
    if (content.contains(QStringLiteral("id=arch")) || content.contains(QStringLiteral("id_like=arch"))) {
        *program = QStandardPaths::findExecutable(QStringLiteral("pacman"));
        if (program->isEmpty()) { *error = QStringLiteral("pacman was not found."); return {}; }
        return {QStringLiteral("-R"), QStringLiteral("--noconfirm"), QStringLiteral("linwin")};
    }
    if (content.contains(QStringLiteral("id=fedora")) || content.contains(QStringLiteral("id_like=fedora"))
        || content.contains(QStringLiteral("id=rhel"))) {
        *program = QStandardPaths::findExecutable(QStringLiteral("dnf"));
        if (program->isEmpty()) { *error = QStringLiteral("dnf was not found."); return {}; }
        return {QStringLiteral("remove"), QStringLiteral("-y"), QStringLiteral("linwin")};
    }
    *program = QStandardPaths::findExecutable(QStringLiteral("apt-get"));
    if (program->isEmpty()) { *error = QStringLiteral("No supported package manager was found. Remove LinWin using your distribution's software manager."); return {}; }
    return {QStringLiteral("remove"), QStringLiteral("-y"), QStringLiteral("linwin")};
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (geteuid() != 0) {
        qCritical("This helper must be started through the system authorization dialog.");
        return 77;
    }
    const QStringList args = app.arguments();
    if (args.size() != 2 || (args.at(1) != QStringLiteral("install-session")
            && args.at(1) != QStringLiteral("install-xfce-deps")
            && args.at(1) != QStringLiteral("uninstall"))) {
        qCritical("Unsupported LinWin helper action.");
        return 64;
    }

    QString error;
    if (args.at(1) == QStringLiteral("install-session")) {
        if (!installSession(&error)) { qCritical().noquote() << error; return 1; }
        return 0;
    }

    if (args.at(1) == QStringLiteral("install-xfce-deps")) {
        if (!installXfceDependencies(&error)) { qCritical().noquote() << error; return 1; }
        return 0;
    }

    if (!removeSession(&error)) { qCritical().noquote() << error; return 1; }
    QString program;
    const QStringList packageArgs = removePackageCommand(&program, &error);
    if (program.isEmpty()) {
        QString restoreError;
        installSession(&restoreError); // Restore the shell choice if package removal cannot start.
        qCritical().noquote() << error;
        return 2;
    }
    const int status = QProcess::execute(program, packageArgs);
    if (status != 0) {
        QString restoreError;
        const bool restored = installSession(&restoreError);
        if (!restored)
            qCritical().noquote() << restoreError;
        qCritical() << (restored ? "Package removal failed; the LinWin session entry was restored."
                                 : "Package removal failed and the LinWin session entry could not be restored.");
        return status == -1 ? 3 : status;
    }
    return 0;
}
