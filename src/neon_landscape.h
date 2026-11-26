// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QPainter>
#include <QImage>
#include "terrain_texture.h"
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPolygonF>
#include <array>
#include <cmath>
#include <cstdint>

namespace LinWinVisual {
inline const QImage &terrainLayer() {
    static const QImage layer=[] {
        constexpr int w=1672,h=882;
        const auto pixels=LinWinTerrain::render(w,h);
        return QImage(reinterpret_cast<const uchar *>(pixels.data()),w,h,w*4,QImage::Format_ARGB32).copy();
    }();
    return layer;
}
inline void drawNeonLandscape(QPainter &p,const QRect &area) {
    if(area.width()<1||area.height()<1)return;
    p.save();p.setRenderHint(QPainter::Antialiasing);p.translate(area.topLeft());p.scale(area.width(),area.height());
    // Wide, luminous folded ribbon; individual strips are vector geometry, not a screenshot.
    QPainterPath ribbon;
    ribbon.moveTo(.23,-.03);ribbon.cubicTo(.43,.20,.55,.27,.71,.30);
    ribbon.cubicTo(.96,.31,.88,.47,.66,.56);ribbon.cubicTo(.50,.62,.49,.57,.72,.47);
    ribbon.cubicTo(.91,.39,.83,.31,.67,.30);ribbon.cubicTo(.50,.28,.39,.14,.34,-.04);ribbon.closeSubpath();
    QLinearGradient ribbonFill(.33,.08,.84,.47);
    ribbonFill.setColorAt(0,QColor(0,65,130,35));ribbonFill.setColorAt(.38,QColor(0,185,255,180));
    ribbonFill.setColorAt(.62,QColor(0,239,255,235));ribbonFill.setColorAt(.84,QColor(0,120,224,125));ribbonFill.setColorAt(1,QColor(0,224,255,220));
    p.setPen(Qt::NoPen);
    for(int i=12;i>0;--i){p.setPen(QPen(QColor(0,180,255,5),.004*i,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawPath(ribbon);}
    p.fillPath(ribbon,ribbonFill);p.setPen(QPen(QColor(93,242,255,190),.0012));p.drawPath(ribbon);
    QPainterPath fold;fold.moveTo(.47,-.04);fold.cubicTo(.53,.17,.60,.26,.71,.30);fold.cubicTo(.91,.35,.87,.40,.69,.51);
    p.setPen(QPen(QColor(147,245,255,135),.0007));p.drawPath(fold);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawImage(QRectF(0,0,1,1),terrainLayer());
    p.restore();
}
}
