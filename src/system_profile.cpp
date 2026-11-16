// SPDX-License-Identifier: GPL-3.0-or-later
#include "system_profile.h"

#include <QFile>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QTextStream>
#include <sys/utsname.h>

namespace {
QString osReleaseValue(const QString &key)
{
    QFile file(QStringLiteral("/etc/os-release"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    const QRegularExpression expression(QStringLiteral("^%1=(.*)$").arg(QRegularExpression::escape(key)));
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const auto match = expression.match(line);
        if (match.hasMatch()) {
            QString value = match.captured(1).trimmed();
            if (value.size() >= 2 && ((value.startsWith('"') && value.endsWith('"')) ||
                                      (value.startsWith('\'') && value.endsWith('\'')))) {
                value = value.mid(1, value.size() - 2);
            }
            return value;
        }
    }
    return {};
}

QString normalizeDesktopName(QString name)
{
    name = name.trimmed();
    if (name.isEmpty()) {
        return QStringLiteral("Unknown desktop environment");
    }

    const QString lower = name.toLower();
    if (lower.contains(QStringLiteral("xfce"))) return QStringLiteral("Xfce");
    if (lower.contains(QStringLiteral("cinnamon"))) return QStringLiteral("Cinnamon");
    if (lower.contains(QStringLiteral("mate"))) return QStringLiteral("MATE");
    if (lower.contains(QStringLiteral("kde")) || lower.contains(QStringLiteral("plasma"))) return QStringLiteral("KDE Plasma");
    if (lower.contains(QStringLiteral("gnome"))) return QStringLiteral("GNOME");
    return name;
}
}

SystemProfile SystemProfile::detect()
{
    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    SystemProfile profile;
    profile.distribution = osReleaseValue(QStringLiteral("PRETTY_NAME"));
    profile.distributionId = osReleaseValue(QStringLiteral("ID"));
    profile.desktopEnvironment = normalizeDesktopName(
        environment.value(QStringLiteral("XDG_CURRENT_DESKTOP"),
                          environment.value(QStringLiteral("DESKTOP_SESSION"))));
    profile.sessionType = environment.value(QStringLiteral("XDG_SESSION_TYPE"), QStringLiteral("Unknown"));

    struct utsname systemInfo {};
    if (uname(&systemInfo) == 0) {
        profile.kernel = QString::fromLocal8Bit(systemInfo.release);
    }
    if (profile.distribution.isEmpty()) {
        profile.distribution = QStringLiteral("Unknown Linux distribution");
    }
    if (profile.distributionId.isEmpty()) {
        profile.distributionId = QStringLiteral("unknown");
    }
    if (profile.kernel.isEmpty()) {
        profile.kernel = QStringLiteral("Unknown");
    }
    return profile;
}
