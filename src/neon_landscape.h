// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <array>
#include <cmath>
namespace LinWinVisual {
inline QPainterPath mountain(const std::array<double,13> &heights, double bottom) {
    QPainterPath path;
    path.moveTo(0,bottom);
    for (std::size_t i=0;i<heights.size();++i)
        path.lineTo(static_cast<double>(i)/static_cast<double>(heights.size()-1),heights[i]);
    path.lineTo(1,bottom); path.closeSubpath();
    return path;
}
// Normalized, deterministic vector landscape shared by CPU and GPU sky paths.
inline void drawNeonLandscape(QPainter &p,const QRect &area) {
    if (area.width()<1 || area.height()<1) return;
    p.save(); p.setRenderHint(QPainter::Antialiasing);
    p.translate(area.topLeft()); p.scale(area.width(),area.height());
    constexpr double horizon=.69;
    const auto far=mountain({.58,.52,.60,.48,.56,.46,.55,.42,.53,.46,.61,.50,.57},horizon);
    const auto near=mountain({.65,.61,.64,.53,.62,.58,.65,.50,.56,.48,.59,.54,.63},horizon);
    QLinearGradient distant(0,.40,0,horizon);
    distant.setColorAt(0,QColor("#3b789a")); distant.setColorAt(1,QColor("#102d4b"));
    p.fillPath(far,distant);
    QLinearGradient foreground(0,.46,0,horizon);
    foreground.setColorAt(0,QColor("#225776")); foreground.setColorAt(1,QColor("#06192f"));
    p.fillPath(near,foreground);
    QLinearGradient water(0,horizon,0,1);
    water.setColorAt(0,QColor("#154e75")); water.setColorAt(.32,QColor("#092b4a")); water.setColorAt(1,QColor("#041426"));
    p.fillRect(QRectF(0,horizon,1,1-horizon),water);
    p.save(); p.setClipRect(QRectF(0,horizon,1,1-horizon));
    p.translate(0,2*horizon); p.scale(1,-1);
    p.setOpacity(.22); p.fillPath(far,QColor("#65a9cb")); p.fillPath(near,QColor("#183e62"));
    p.restore();
    // Fine highlights in the lake, with deterministic widths and positions.
    p.setPen(Qt::NoPen);
    for (int i=0;i<34;++i) {
        const double f=static_cast<double>(i)/34.0;
        const double y=horizon+.012+.27*f*f;
        const double x=.55+.22*std::sin(i*2.37);
        const double w=.035+.19*f;
        QLinearGradient ripple(x-w,y,x+w,y);
        ripple.setColorAt(0,QColor(40,195,245,0));
        ripple.setColorAt(.5,QColor(40,195,245,static_cast<int>(55*(1-f))));
        ripple.setColorAt(1,QColor(40,195,245,0));
        p.fillRect(QRectF(x-w,y,w*2,.0016+.001*f),ripple);
    }
    QPainterPath ribbon;
    ribbon.moveTo(-.08,.37); ribbon.cubicTo(.22,-.01,.56,.02,1.1,.40);
    QPainterPath second;
    second.moveTo(-.05,.40); second.cubicTo(.32,.05,.62,.12,1.1,.30);
    for (int i=5;i>=1;--i) {
        p.setPen(QPen(QColor(20,190,255,i==1?120:10),.0025*i,Qt::SolidLine,Qt::RoundCap));
        p.drawPath(ribbon); p.drawPath(second);
    }
    p.restore();
}
}
