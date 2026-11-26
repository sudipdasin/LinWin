// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
namespace LinWinVisual {
inline QIcon placeIcon(const QString &place) {
    QPixmap pix(96,96);pix.fill(Qt::transparent);
    QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing);p.scale(2,2);
    auto gradient=[](QColor top,QColor bottom){QLinearGradient g(0,4,0,44);g.setColorAt(0,top);g.setColorAt(1,bottom);return g;};
    p.setPen(Qt::NoPen);
    if(place=="computer"){
        p.setBrush(gradient(QColor("#6be8fc"),QColor("#0065cc")));p.drawRoundedRect(QRectF(3,5,42,30),3,3);
        p.setBrush(gradient(QColor("#dcebf6"),QColor("#67849d")));p.drawRect(QRectF(20,35,8,7));p.drawRoundedRect(QRectF(12,42,24,3),1,1);
        p.setPen(QPen(QColor("#b9f6ff"),.7));p.drawLine(QPointF(6,7),QPointF(42,7));
    } else if(place=="home"){
        QPainterPath folder;folder.moveTo(3,10);folder.lineTo(20,10);folder.lineTo(25,15);folder.lineTo(45,15);folder.lineTo(42,42);folder.lineTo(3,42);folder.closeSubpath();
        p.setBrush(gradient(QColor("#ffdd80"),QColor("#e9a340")));p.drawPath(folder);
        p.setBrush(gradient(QColor("#ffe8a7"),QColor("#ffbb4b")));p.drawRoundedRect(QRectF(3,18,42,24),3,3);
        QPainterPath house;house.moveTo(15,28);house.lineTo(24,20);house.lineTo(33,28);house.lineTo(30,28);house.lineTo(30,38);house.lineTo(26,38);house.lineTo(26,32);house.lineTo(22,32);house.lineTo(22,38);house.lineTo(18,38);house.lineTo(18,28);house.closeSubpath();p.fillPath(house,QColor("#a96d2e"));
    } else if(place=="trash"){
        QPainterPath bin;bin.moveTo(10,14);bin.lineTo(38,14);bin.lineTo(34,44);bin.lineTo(14,44);bin.closeSubpath();
        p.setBrush(gradient(QColor("#e8fbff"),QColor("#7799b3")));p.drawPath(bin);
        p.setBrush(QColor("#bcd3e4"));p.drawRoundedRect(QRectF(8,9,32,5),2,2);p.drawRoundedRect(QRectF(18,5,12,5),2,2);
        p.setPen(QPen(QColor("#0a8ec7"),2,Qt::SolidLine,Qt::RoundCap));p.drawArc(QRectF(16,22,16,16),35*16,250*16);
        p.drawLine(QPointF(31,24),QPointF(32,31));p.drawLine(QPointF(32,31),QPointF(25,29));
    } else {
        p.setBrush(gradient(QColor("#46ceff"),QColor("#0659c4")));p.drawEllipse(QRectF(3,3,42,42));
        p.setPen(QPen(QColor("#e0f8ff"),1.3));p.setBrush(Qt::NoBrush);p.drawEllipse(QRectF(14,3,20,42));p.drawLine(QPointF(4,24),QPointF(44,24));p.drawArc(QRectF(4,8,40,17),180*16,180*16);p.drawArc(QRectF(4,23,40,17),0,180*16);
    }
    p.end();return QIcon(pix);
}
}
