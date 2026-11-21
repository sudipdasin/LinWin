// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QList>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

struct WorkspaceInfo {
    int index = 0;
    QString name;
    bool current = false;
};

inline QList<WorkspaceInfo> parseWorkspaceList(const QString &output)
{
    static const QRegularExpression prefix(QStringLiteral("^\\s*(\\d+)\\s+([*-])(?:\\s|$)"));
    QList<WorkspaceInfo> workspaces;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const auto match = prefix.match(line);
        if (!match.hasMatch()) continue;

        bool validIndex = false;
        const int index = match.captured(1).toInt(&validIndex);
        if (!validIndex) continue;

        const int nameMarker = line.indexOf(QStringLiteral(" N/A "), match.capturedEnd() - 1);
        QString name = nameMarker >= 0 ? line.mid(nameMarker + 5).trimmed() : QString();
        if (name.isEmpty()) name = QObject::tr("Desktop %1").arg(index + 1);
        workspaces.append({index, name, match.captured(2) == QStringLiteral("*")});
    }
    return workspaces;
}
