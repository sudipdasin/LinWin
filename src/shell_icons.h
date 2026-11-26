// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPixmap>
#include <QPolygonF>
namespace LinWinVisual {
enum class Glyph { Files, Tasks, Settings, Clipboard, Desktop, Search, Snap, More };
inline QIcon shellIcon(Glyph glyph) {
    QIcon icon;
    for (int size : {24,32,48,96}) {
        QPixmap pm(size,size); pm.fill(Qt::transparent);
        QPainter p(&pm); p.setRenderHint(QPainter::Antialiasing); p.scale(size/48.0,size/48.0);
        QLinearGradient bg(0,0,48,48); bg.setColorAt(0,QColor("#3679b3")); bg.setColorAt(1,QColor("#163d70"));
        p.setPen(QPen(QColor("#7dc9f4"),1)); p.setBrush(bg); p.drawRoundedRect(QRectF(3,3,42,42),9,9);
        p.setPen(QPen(QColor("#f3faff"),2.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
        switch(glyph) {
        case Glyph::Files: {
            QPainterPath folder; folder.moveTo(9,16); folder.lineTo(20,16); folder.lineTo(24,20); folder.lineTo(39,20); folder.lineTo(37,36); folder.lineTo(10,36); folder.closeSubpath();
            p.setPen(QPen(QColor("#e1a843"),1.5)); p.setBrush(QColor("#ffdd88")); p.drawPath(folder); p.setPen(QPen(QColor("#fff0be"),2)); p.drawLine(QPointF(12,23),QPointF(36,23)); break;
        }
        case Glyph::Tasks: p.drawRoundedRect(QRectF(10,12,28,24),3,3); p.drawPolyline(QPolygonF{QPointF(13,25),QPointF(19,25),QPointF(23,17),QPointF(28,31),QPointF(32,23),QPointF(35,23)}); break;
        case Glyph::Settings:
            p.drawEllipse(QPointF(24,24),10,10); p.drawEllipse(QPointF(24,24),4,4);
            for(int i=0;i<8;++i){p.save(); p.translate(24,24); p.rotate(i*45); p.drawLine(QPointF(0,-11),QPointF(0,-15)); p.restore();} break;
        case Glyph::Clipboard: p.drawRoundedRect(QRectF(13,12,22,26),3,3); p.setBrush(QColor("#3679b3")); p.drawRoundedRect(QRectF(19,9,10,7),2,2); p.drawLine(18,23,30,23); p.drawLine(18,29,28,29); break;
        case Glyph::Desktop: p.drawRoundedRect(QRectF(10,12,28,21),3,3); p.drawLine(24,33,24,38); p.drawLine(17,38,31,38); break;
        case Glyph::Search: p.drawEllipse(QPointF(21,21),9,9); p.drawLine(28,28,36,36); break;
        case Glyph::Snap: p.drawRoundedRect(QRectF(10,12,28,24),3,3); p.drawLine(24,13,24,35); p.drawLine(25,24,37,24); break;
        case Glyph::More: p.setBrush(QColor("#f3faff")); p.setPen(Qt::NoPen); for(int x:{15,24,33})p.drawEllipse(QPointF(x,24),2,2); break;
        }
        p.end(); icon.addPixmap(pm);
    }
    return icon;
}
}
