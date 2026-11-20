// SPDX-License-Identifier: GPL-3.0-or-later
#include "xfce_dependency_plan.h"

#include <iostream>

namespace {
bool expect(bool condition, const char *message)
{
    if (!condition) std::cerr << message << '\n';
    return condition;
}
}

int main()
{
    bool passed = true;
    passed &= expect(packageManagerForOsRelease(QStringLiteral("ID=ubuntu\nID_LIKE=debian\n")) == PackageManager::Apt,
                     "Ubuntu should use APT");
    passed &= expect(packageManagerForOsRelease(QStringLiteral("ID=linuxmint\n")) == PackageManager::Apt,
                     "Linux Mint should use APT");
    passed &= expect(packageManagerForOsRelease(QStringLiteral("ID=rocky\nID_LIKE=\"rhel fedora\"\n")) == PackageManager::Dnf,
                     "RHEL-compatible distributions should use DNF");
    passed &= expect(packageManagerForOsRelease(QStringLiteral("ID=manjaro\nID_LIKE=arch\n")) == PackageManager::Pacman,
                     "Manjaro should use pacman");
    passed &= expect(packageManagerForOsRelease(QStringLiteral("ID=alpine\nID_LIKE=\n")) == PackageManager::Unsupported,
                     "Unsupported distributions must not guess a package manager");

    const QStringList aptArgs = xfceDependencyInstallArguments(PackageManager::Apt);
    passed &= expect(aptArgs == QStringList{QStringLiteral("-y"), QStringLiteral("-o"),
        QStringLiteral("Dpkg::Use-Pty=0"), QStringLiteral("--no-install-recommends"),
        QStringLiteral("install"), QStringLiteral("xfwm4"), QStringLiteral("xfce4-settings"),
        QStringLiteral("xfdesktop4")}, "APT plan should be fixed and avoid recommended extras");
    passed &= expect(xfceDependencyInstallArguments(PackageManager::Dnf) == QStringList{
        QStringLiteral("install"), QStringLiteral("-y"), QStringLiteral("--setopt=install_weak_deps=False"),
        QStringLiteral("xfwm4"), QStringLiteral("xfce4-settings"), QStringLiteral("xfdesktop")},
        "DNF plan should install only the XFCE session packages and their required dependencies");
    passed &= expect(xfceDependencyInstallArguments(PackageManager::Pacman) == QStringList{
        QStringLiteral("-S"), QStringLiteral("--noconfirm"), QStringLiteral("--needed"),
        QStringLiteral("xfwm4"), QStringLiteral("xfce4-settings"), QStringLiteral("xfdesktop")},
        "pacman plan should be fixed, noninteractive, and avoid reinstalling present packages");
    passed &= expect(xfceDependencyInstallArguments(PackageManager::Unsupported).isEmpty(),
                     "Unsupported distributions must not receive a package command");
    return passed ? 0 : 1;
}
