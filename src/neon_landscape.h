// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPolygonF>
#include <array>
#include <cmath>
#include <cstdint>

namespace LinWinVisual {
inline double terrainNoise(int i, int seed) {
    std::uint32_t n=static_cast<std::uint32_t>(i)*374761393u+static_cast<std::uint32_t>(seed)*668265263u;
    n=(n^(n>>13))*1274126177u;
    return static_cast<double>(n^(n>>16))/4294967295.0;
}
inline QPolygonF ridge(const std::array<double,13> &height,int seed,double amplitude) {
    QPolygonF points;
    for(int i=0;i<=256;++i) {
        const double x=static_cast<double>(i)/256.0;
        const double f=x*12;
        const int a=qMin(11,static_cast<int>(f));
        const double y=height[static_cast<std::size_t>(a)]*(1-(f-a))+height[static_cast<std::size_t>(a+1)]*(f-a);
        const double edge=std::sin(3.141592653589793*(f-a));
        const double detail=(terrainNoise(i,seed)-.5)*amplitude+std::sin(i*.72)*amplitude*.35;
        points.append(QPointF(x,y+detail*edge));
    }
    return points;
}
inline QPainterPath mountainPath(const QPolygonF &points,double bottom) {
    QPainterPath path; path.moveTo(0,bottom);
    for(const auto &point:points) path.lineTo(point);
    path.lineTo(1,bottom);path.closeSubpath();return path;
}
inline void drawNeonLandscape(QPainter &p,const QRect &area) {
    if(area.width()<1||area.height()<1)return;
    p.save();p.setRenderHint(QPainter::Antialiasing);p.translate(area.topLeft());p.scale(area.width(),area.height());
    constexpr double horizon=.795;
    // Wide, luminous folded ribbon; individual strips are vector geometry, not a screenshot.
    QPainterPath ribbon;
    ribbon.moveTo(.23,-.03);ribbon.cubicTo(.43,.20,.55,.27,.71,.30);
    ribbon.cubicTo(.96,.31,.88,.47,.66,.56);ribbon.cubicTo(.50,.62,.49,.57,.72,.47);
    ribbon.cubicTo(.91,.39,.83,.31,.67,.30);ribbon.cubicTo(.50,.28,.39,.14,.34,-.04);ribbon.closeSubpath();
    QLinearGradient ribbonFill(.33,.08,.84,.47);
    ribbonFill.setColorAt(0,QColor(0,65,130,35));ribbonFill.setColorAt(.38,QColor(0,185,255,180));
    ribbonFill.setColorAt(.62,QColor(0,239,255,235));ribbonFill.setColorAt(.84,QColor(0,120,224,45));ribbonFill.setColorAt(1,QColor(0,224,255,220));
    p.setPen(Qt::NoPen);
    for(int i=12;i>0;--i){p.setPen(QPen(QColor(0,180,255,5),.004*i,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawPath(ribbon);}
    p.fillPath(ribbon,ribbonFill);p.setPen(QPen(QColor(93,242,255,190),.0012));p.drawPath(ribbon);
    QPainterPath fold;fold.moveTo(.47,-.04);fold.cubicTo(.53,.17,.60,.26,.71,.30);fold.cubicTo(.91,.35,.87,.40,.69,.51);
    p.setPen(QPen(QColor(210,255,255,220),.001));p.drawPath(fold);
    const auto far=ridge({.77,.72,.69,.65,.68,.62,.67,.61,.58,.54,.62,.67,.71},17,.018);
    const auto near=ridge({.78,.74,.69,.65,.70,.72,.71,.65,.60,.56,.64,.68,.72},43,.026);
    auto layer=[&](const QPolygonF &points,QColor top,QColor base,int seed){
        const auto path=mountainPath(points,horizon);
        QLinearGradient grad(0,.53,0,horizon);grad.setColorAt(0,top);grad.setColorAt(1,base);
        p.fillPath(path,grad);p.save();p.setClipPath(path);
        // Multiscale ridges and gullies along the slope, deterministic and resolution independent.
        for(int i=2;i<points.size()-2;++i) {
            const auto a=points[i];const double depth=horizon-a.y();
            const double dx=(terrainNoise(i,seed+3)-.5)*.09;
            const double width=.0015+terrainNoise(i,seed+8)*.004;
            QPainterPath facet;facet.moveTo(a);facet.lineTo(a.x()+width,a.y()+depth*.13);
            facet.lineTo(a.x()+dx,horizon);facet.closeSubpath();
            const int light=static_cast<int>(terrainNoise(i,seed)*100);
            p.fillPath(facet,QColor(5+light/5,37+light/3,75+light/2,120));
            if(i%3==0){p.setPen(QPen(QColor(28,167,244,70+light),.00065));p.drawLine(a,QPointF(a.x()+dx*.70,a.y()+depth*.64));}
        }
        p.restore();
    };
    layer(far,QColor("#195ca4"),QColor("#071c36"),31);
    layer(near,QColor("#1266b3"),QColor("#031429"),81);
    QLinearGradient water(0,horizon,0,1);
    water.setColorAt(0,QColor("#084e83"));water.setColorAt(.18,QColor("#092746"));water.setColorAt(1,QColor("#031327"));
    p.fillRect(QRectF(0,horizon,1,1-horizon),water);
    p.save();p.setClipRect(QRectF(0,horizon,1,1-horizon));
    p.translate(0,2*horizon);p.scale(1,-1);p.setOpacity(.24);
    p.fillPath(mountainPath(far,horizon),QColor("#2378b6"));p.fillPath(mountainPath(near,horizon),QColor("#0b335b"));p.restore();
    QRadialGradient reflection(QPointF(.64,horizon),.25);reflection.setColorAt(0,QColor(0,225,255,150));reflection.setColorAt(1,QColor(0,110,220,0));
    p.fillRect(QRectF(0,horizon,1,1-horizon),reflection);
    p.setPen(Qt::NoPen);
    for(int i=0;i<180;++i) {
        const double f=static_cast<double>(i)/180.0;
        const double y=horizon+.001+(1-horizon-.002)*f;
        const double x=.62+(terrainNoise(i,71)-.5)*.42;
        const double w=.025+terrainNoise(i,31)*(.10+.15*f);
        QLinearGradient ripple(x-w,y,x+w,y);
        ripple.setColorAt(0,QColor(0,130,235,0));ripple.setColorAt(.5,QColor(0,202,255,static_cast<int>((35+terrainNoise(i,89)*110)*(1-f*.65))));ripple.setColorAt(1,QColor(0,110,210,0));
        p.fillRect(QRectF(x-w,y,w*2,.0006+.0009*f),ripple);
    }
    p.setPen(QPen(QColor(38,221,255,145),.0011));p.drawLine(QPointF(.35,horizon),QPointF(.89,horizon));
    p.restore();
}
}
