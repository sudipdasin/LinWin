// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "theme_catalog.h"
#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QIcon>
#include <QPixmap>
namespace LinWinVisual {
inline QString qstr(std::string_view s) { return QString::fromLatin1(s.data(), static_cast<qsizetype>(s.size())); }
inline const LinWinTheme::Theme &theme(const QString &id) { return LinWinTheme::byId(id.toStdString()); }
inline void drawLogo(QPainter &p, QRectF r) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(r.topLeft());
    p.scale(r.width()/100.0, r.height()/100.0);
    QPainterPath loop;
    loop.moveTo(24, 15); loop.cubicTo(29, 8, 36, 10, 44, 15);
    loop.lineTo(83, 40); loop.cubicTo(96, 49, 92, 61, 81, 67);
    loop.lineTo(32, 91); loop.cubicTo(17, 99, 6, 87, 10, 71);
    loop.lineTo(22, 22); loop.closeSubpath();
    QLinearGradient g(0, 0, 100, 100);
    g.setColorAt(0, QColor("#08edff")); g.setColorAt(.52, QColor("#158be8")); g.setColorAt(1, QColor("#01f2de"));
    QPen pen(QBrush(g), 11, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen); p.setBrush(Qt::NoBrush); p.drawPath(loop);
    p.restore();
}
inline QIcon logoIcon() {
    QPixmap pix(96,96); pix.fill(Qt::transparent);
    QPainter p(&pix); drawLogo(p, QRectF(8,8,80,80)); p.end();
    return QIcon(pix);
}
inline QString shellStyle(const LinWinTheme::Theme &t) {
    const QString panel=qstr(t.panel), fg=qstr(t.text), accent=qstr(t.accent);
    const QString field=t.light ? QStringLiteral("#ffffff") : QStringLiteral("#122e49");
    const QString hover=t.light ? QStringLiteral("#d7e5f6") : QStringLiteral("#214563");
    const QString radius=t.effects ? QStringLiteral("9px") : QStringLiteral("0px");
    const QString subtle=t.light ? QStringLiteral("#b8c9db") : QStringLiteral("#365575");
    const QString muted=t.light ? QStringLiteral("#425773") : QStringLiteral("#b9d3e9");
    return QStringLiteral(
        "#taskbar, #startMenuPanel { background:%1; color:%2; border:1px solid %7; border-radius:%4; }"
        "#taskbar QWidget, #startMenuPanel QLabel { background:transparent; color:%2; }"
        "#taskbar QPushButton, #startMenuPanel QPushButton { color:%2; background:transparent; border:0; padding:6px; }"
        "#taskbar QPushButton:hover, #taskbar QPushButton:checked, #startMenuPanel QPushButton:hover { background:%5; border-bottom:2px solid %3; }"
        "#taskbar QPushButton:focus, #startMenuPanel QPushButton:focus { border:1px solid %3; }"
        "#startMenuPanel #intro, #startMenuPanel #launcherStatus { color:%8; font-size:12px; }"
        "#startMenuPanel QCheckBox { color:%2; background:transparent; }"
        "#startMenuPanel QListWidget::item { padding:5px; border-radius:%4; }"
        "#startMenuPanel QListWidget::item:hover { background:%5; }"
        "#startMenuPanel #applicationSearch:focus { border:2px solid %3; }"
        "QMenu { background:%1; color:%2; border:1px solid %7; padding:5px; }"
        "QMenu::item { padding:7px 24px; border-radius:%4; }"
        "QMenu::item:selected { background:%5; color:%2; }"
        "QMenu::item:disabled { color:%8; }"
        "QMenu::separator { height:1px; background:%7; margin:4px 8px; }"
        "#linwinSnapMenu { background:%1; color:%2; border:1px solid %7; border-radius:%4; padding:5px 10px; }"
        "#linwinSnapMenu:hover { background:%5; border-color:%3; }"
        "#startMenuPanel #applicationSearch, #startMenuPanel #applicationList, #startMenuPanel #pinnedApps, #startMenuPanel #recentDocuments { background:%6; color:%2; border:1px solid %7; border-radius:%4; }"
        "#startMenuPanel QListWidget::item:selected { background:%5; color:%2; }"
        "#startMenuPanel #primaryAction { background:%5; color:%2; border:1px solid %3; border-radius:%4; }"
        "#taskbar #startButton, #taskbar #panelClock, #taskbar #panelStatus, #taskbar #windowSwitcherStatus { color:%2; }"
        "#taskbar #windowSwitcher QPushButton { background:%5; color:%2; border-bottom:2px solid %3; }"
    ).arg(panel,fg,accent,radius,hover,field,subtle,muted);
}
}
