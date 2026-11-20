// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QString>
#include <QStringList>
#include <QRegularExpression>

enum class PackageManager { Apt, Dnf, Pacman, Unsupported };

inline PackageManager packageManagerForOsRelease(const QString &contents)
{
    QString id;
    QStringList idLike;
    const QStringList lines = contents.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const qsizetype separator = line.indexOf(QLatin1Char('='));
        if (separator <= 0) continue;
        const QString key = line.left(separator).trimmed();
        QString value = line.mid(separator + 1).trimmed();
        if (value.size() >= 2 && value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))
            value = value.mid(1, value.size() - 2);
        else if (value.size() >= 2 && value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))
            value = value.mid(1, value.size() - 2);
        if (key == QStringLiteral("ID")) id = value.toLower();
        else if (key == QStringLiteral("ID_LIKE")) idLike = value.toLower().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    }

    if (id == QStringLiteral("arch") || id == QStringLiteral("manjaro") || id == QStringLiteral("endeavouros"))
        return PackageManager::Pacman;
    if (id == QStringLiteral("fedora") || id == QStringLiteral("rhel") || id == QStringLiteral("centos") || id == QStringLiteral("rocky") || id == QStringLiteral("almalinux"))
        return PackageManager::Dnf;
    if (id == QStringLiteral("debian") || id == QStringLiteral("ubuntu") || id == QStringLiteral("linuxmint") || id == QStringLiteral("pop"))
        return PackageManager::Apt;
    if (idLike.contains(QStringLiteral("arch"))) return PackageManager::Pacman;
    if (idLike.contains(QStringLiteral("fedora")) || idLike.contains(QStringLiteral("rhel"))) return PackageManager::Dnf;
    if (idLike.contains(QStringLiteral("debian")) || idLike.contains(QStringLiteral("ubuntu"))) return PackageManager::Apt;
    return PackageManager::Unsupported;
}

inline QString packageManagerProgram(PackageManager manager)
{
    switch (manager) {
    case PackageManager::Apt: return QStringLiteral("apt-get");
    case PackageManager::Dnf: return QStringLiteral("dnf");
    case PackageManager::Pacman: return QStringLiteral("pacman");
    case PackageManager::Unsupported: return {};
    }
    return {};
}

inline QStringList xfceDependencyPackages(PackageManager manager)
{
    if (manager == PackageManager::Apt)
        return {QStringLiteral("xfwm4"), QStringLiteral("xfce4-settings"), QStringLiteral("xfdesktop4")};
    if (manager == PackageManager::Dnf || manager == PackageManager::Pacman)
        return {QStringLiteral("xfwm4"), QStringLiteral("xfce4-settings"), QStringLiteral("xfdesktop")};
    return {};
}

inline QStringList xfceDependencyInstallArguments(PackageManager manager)
{
    QStringList arguments;
    switch (manager) {
    case PackageManager::Apt:
        arguments = {QStringLiteral("-y"), QStringLiteral("-o"), QStringLiteral("Dpkg::Use-Pty=0"),
                     QStringLiteral("--no-install-recommends"), QStringLiteral("install")};
        break;
    case PackageManager::Dnf:
        arguments = {QStringLiteral("install"), QStringLiteral("-y"), QStringLiteral("--setopt=install_weak_deps=False")};
        break;
    case PackageManager::Pacman:
        arguments = {QStringLiteral("-S"), QStringLiteral("--noconfirm"), QStringLiteral("--needed")};
        break;
    case PackageManager::Unsupported:
        return {};
    }
    arguments.append(xfceDependencyPackages(manager));
    return arguments;
}
