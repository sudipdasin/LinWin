// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QRegularExpression>
#include <QString>
#include <QtGlobal>

inline QString linwinScaledStyleSheet(const QString &styleSheet, int percent)
{
    const QRegularExpression expression(QStringLiteral("font-size\\s*:\\s*(\\d+(?:\\.\\d+)?)px"));
    const auto matches = expression.globalMatch(styleSheet);
    QString result;
    qsizetype copiedUntil = 0;
    for (auto it = matches; it.hasNext();) {
        const QRegularExpressionMatch match = it.next();
        bool ok = false;
        const double baseSize = match.captured(1).toDouble(&ok);
        if (!ok) continue;
        result += styleSheet.mid(copiedUntil, match.capturedStart() - copiedUntil);
        result += QStringLiteral("font-size: %1px").arg(qMax(1, qRound(baseSize * percent / 100.0)));
        copiedUntil = match.capturedEnd();
    }
    result += styleSheet.mid(copiedUntil);
    return result;
}
