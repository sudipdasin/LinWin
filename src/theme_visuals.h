// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "theme_catalog.h"
#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QIcon>
#include <QPixmap>
#include <QSettings>
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
    p.setBrush(Qt::NoBrush);
    for(int i=5;i>=1;--i){p.setPen(QPen(QColor(0,182,255,9),15+i*3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawPath(loop);}
    p.setPen(QPen(QColor("#075397"),19,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawPath(loop);
    p.setPen(QPen(QBrush(g),16,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawPath(loop);
    p.setPen(QPen(QColor(188,255,255,190),1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    QPainterPath highlight;highlight.moveTo(13,69);highlight.lineTo(24,19);highlight.cubicTo(27,11,35,9,45,16);highlight.lineTo(83,41);p.drawPath(highlight);
    p.restore();
}
inline QIcon logoIcon() {
    QPixmap pix(96,96); pix.fill(Qt::transparent);
    QPainter p(&pix); drawLogo(p, QRectF(8,8,80,80)); p.end();
    return QIcon(pix);
}
inline QString shellStyle(const LinWinTheme::Theme &t) {
    QSettings settings;
    const QString mode=settings.value("visual/shellMode","theme").toString();
    const bool light=mode=="light"||(mode!="dark"&&t.light);
    const QString panel=mode=="light"?"#edf2f8":mode=="dark"?"#17222f":qstr(t.panel);
    const QString fg=mode=="light"?"#172940":mode=="dark"?"#edf2f8":qstr(t.text);
    const QColor custom(settings.value("visual/accent").toString());
    const QString accent=custom.isValid()?custom.name():qstr(t.accent);
    const QString field=light ? QStringLiteral("#ffffff") : QStringLiteral("#122e49");
    const QString hover=light ? QStringLiteral("#d7e5f6") : QStringLiteral("#214563");
    const QString radius=t.effects ? QStringLiteral("9px") : QStringLiteral("0px");
    const QString subtle=light ? QStringLiteral("#b8c9db") : QStringLiteral("#365575");
    const QString muted=light ? QStringLiteral("#425773") : QStringLiteral("#b9d3e9");
    QString style=QStringLiteral(
        "#taskbar, #startMenuPanel { background:%1; color:%2; border:1px solid %7; border-radius:%4; }"
        "#taskbar QWidget, #startMenuPanel QLabel, #linwinStartViews, #linwinStartViews QWidget, #linwinStartRail { background:transparent; color:%2; }"
        "#startMenuPanel #linwinStartRail QPushButton { text-align:left; padding:9px 6px; }"
        "#startMenuPanel #linwinStartRail { border-right:1px solid %7; }"
        "#startMenuPanel #linwinStartFooter { border-top:1px solid %7; background:%1; }"
        "#startMenuPanel #launcherTagline { color:%8; font-size:11px; }"
        "#startMenuPanel #linwinStartSection { font-size:15px; font-weight:600; color:%2; }"
        "#startMenuPanel #linwinStartRail QPushButton { font-size:14px; }"
        "#startMenuPanel #pinnedApps { border:0; background:transparent; }"
        "#startMenuPanel #linwinAllApps, #startMenuPanel #linwinPinnedView { border-radius:6px; }"
        "#startMenuPanel #linwinStartRail QPushButton:checked { background:%5; color:%2; border:1px solid %7; }"
        "#startMenuPanel #pageTitle { color:%2; font-size:24px; font-weight:600; }"
        "#startMenuPanel QScrollBar:vertical { background:%1; width:7px; margin:0; }"
        "#startMenuPanel QScrollBar::handle:vertical { background:%7; border-radius:3px; min-height:18px; }"
        "#startMenuPanel QScrollBar::add-line:vertical, #startMenuPanel QScrollBar::sub-line:vertical { height:0; }"
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
        "#startMenuPanel #applicationSearch, #startMenuPanel #applicationList, #startMenuPanel #recentDocuments { background:%6; color:%2; border:1px solid %7; border-radius:%4; }"
        "#startMenuPanel #pinnedApps { background:transparent; color:%2; border:0; }"
        "#startMenuPanel #pinnedApps::item { padding:2px; }"
        "#taskbar #linwinStatusArea QPushButton { padding:2px; border-radius:6px; }"
        "#startMenuPanel QListWidget::item:selected { background:%5; color:%2; }"
        "#startMenuPanel #primaryAction { background:%5; color:%2; border:1px solid %3; border-radius:%4; }"
        "#taskbar #startButton, #taskbar #panelClock, #taskbar #panelStatus, #taskbar #windowSwitcherStatus { color:%2; }"
        "#taskbar #linwinTaskbarSearch { background:%6; border:1px solid %7; border-radius:14px; padding:4px 10px; text-align:left; }"
        "#taskbar #linwinTaskbarSearch:hover { border-color:%3; }"
        "#taskbar QScrollBar:horizontal { height:4px; background:%1; }"
        "#taskbar QScrollBar::handle:horizontal { background:%7; min-width:16px; }"
        "#taskbar QScrollBar::add-line:horizontal, #taskbar QScrollBar::sub-line:horizontal { width:0; }"
        "#taskbar #windowSwitcher QPushButton { background:transparent; color:%2; border-radius:6px; border-bottom:2px solid %3; padding:4px; }"
        "#taskbar #windowSwitcher QPushButton:hover, #taskbar #windowSwitcher QPushButton:focus { background:%5; }"
    ).arg(panel,fg,accent,radius,hover,field,subtle,muted);
    for(const QString &name: {QStringLiteral("linwinCommandPalette"),QStringLiteral("linwinQuickControls"),QStringLiteral("linwinWindowOverview")}) {
        const QString scope=QStringLiteral("#")+name;
        style+=QStringLiteral(
            "%1 { background:%2; color:%3; }"
            "%1 QLabel, %1 QCheckBox { color:%3; background:transparent; }"
            "%1 QLineEdit, %1 QListWidget, %1 QComboBox { color:%3; background:%4; border:1px solid %5; border-radius:%6; padding:6px; }"
            "%1 QLineEdit:focus, %1 QComboBox:focus, %1 QListWidget:focus { border-color:%7; }"
            "%1 QListWidget::item { padding:5px; border-radius:%6; }"
            "%1 QListWidget::item:hover, %1 QListWidget::item:selected { background:%8; color:%3; }"
            "%1 QPushButton { color:%3; background:%8; border:1px solid %5; border-radius:%6; padding:7px 14px; }"
            "%1 QPushButton:hover, %1 QPushButton:focus { border-color:%7; }"
            "%1 QPushButton:disabled, %1 QCheckBox:disabled { color:%9; background:%2; }"
        ).arg(scope,panel,fg,field,subtle,radius,accent,hover,muted);
    }
    return style;
}
}
