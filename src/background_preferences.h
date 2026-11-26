// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>
#include <QSettings>
#include <QStringList>
namespace LinWinBackground {
struct Preferences {
    QString mode="theme",picture,folder,fit="fill";
    QColor color=QColor("#10243b");
    int minutes=10;
};
inline Preferences load(QSettings &s) {
    Preferences p;
    p.mode=s.value("background/mode","theme").toString();
    if(!QStringList{"theme","picture","solid","slideshow"}.contains(p.mode))p.mode="theme";
    p.picture=s.value("background/picture").toString();
    p.folder=s.value("background/folder").toString();
    p.fit=s.value("background/fit","fill").toString();
    if(!QStringList{"fill","fit","stretch","tile","center","span"}.contains(p.fit))p.fit="fill";
    const QColor color(s.value("background/color","#10243b").toString());
    if(color.isValid())p.color=color;
    p.minutes=qBound(1,s.value("background/minutes",10).toInt(),1440);
    return p;
}
inline QImage readImage(const QString &path) {
    QFileInfo file(path);
    if(!file.isFile()||!file.isReadable()||file.size()>64LL*1024*1024)return {};
    QImageReader reader(path);reader.setAutoTransform(true);
    const QSize size=reader.size();
    if(!size.isValid()||qint64(size.width())*size.height()>64000000LL)return {};
    if(size.width()>4096||size.height()>4096)reader.setScaledSize(size.scaled(4096,4096,Qt::KeepAspectRatio));
    return reader.read();
}
inline QStringList slides(const QString &folder) {
    if(folder.isEmpty()||!QDir(folder).exists())return {};
    QDir d(folder);QStringList out;
    const auto formats=QImageReader::supportedImageFormats();
    for(const auto &file:d.entryInfoList(QDir::Files|QDir::Readable,QDir::Name|QDir::IgnoreCase)){
        if(formats.contains(file.suffix().toLower().toLatin1())&&file.size()<=64LL*1024*1024)out.append(file.absoluteFilePath());
        if(out.size()>=1000)break;
    }
    return out;
}
inline bool save(QSettings &s,const Preferences &p) {
    if(!QStringList{"theme","picture","solid","slideshow"}.contains(p.mode)||
        !QStringList{"fill","fit","stretch","tile","center","span"}.contains(p.fit)||!p.color.isValid()||p.minutes<1||p.minutes>1440)return false;
    if(p.mode=="picture"&&readImage(p.picture).isNull())return false;
    if(p.mode=="slideshow"){
        bool valid=false;for(const auto &path:slides(p.folder))if(!readImage(path).isNull()){valid=true;break;}
        if(!valid)return false;
    }
    s.setValue("background/mode",p.mode);s.setValue("background/picture",p.picture);s.setValue("background/folder",p.folder);
    s.setValue("background/fit",p.fit);s.setValue("background/color",p.color.name());s.setValue("background/minutes",p.minutes);
    if(p.mode=="picture"){
        auto recent=s.value("background/recent").toStringList();recent.removeAll(p.picture);recent.prepend(p.picture);
        while(recent.size()>8)recent.removeLast();s.setValue("background/recent",recent);
    }
    s.sync();return s.status()==QSettings::NoError;
}
// Canvas and screen region are logical desktop coordinates. Span crops one
// virtual-desktop-sized fill to the current surface, rather than stretching.
inline void paint(QPainter &p,const QRect &area,const QImage &image,const Preferences &prefs,
                  const QRect &virtualCanvas={},const QRect &surfaceGlobal={}) {
    p.save();p.setClipRect(area);p.fillRect(area,prefs.color);
    if(image.isNull()||area.isEmpty()){p.restore();return;}
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    if(prefs.fit=="tile"){
        for(int y=area.top();y<=area.bottom();y+=image.height())
            for(int x=area.left();x<=area.right();x+=image.width())p.drawImage(QPoint(x,y),image);
    }else{
        QRect target=area;
        if(prefs.fit=="span"&&virtualCanvas.isValid()&&surfaceGlobal.isValid()){
            const qreal sx=qreal(area.width())/surfaceGlobal.width(),sy=qreal(area.height())/surfaceGlobal.height();
            target=QRect(area.x()+qRound((virtualCanvas.x()-surfaceGlobal.x())*sx),area.y()+qRound((virtualCanvas.y()-surfaceGlobal.y())*sy),qRound(virtualCanvas.width()*sx),qRound(virtualCanvas.height()*sy));
        }
        if(prefs.fit!="stretch"){
            QSize size=image.size();
            if(prefs.fit!="center")size.scale(target.size(),prefs.fit=="fit"?Qt::KeepAspectRatio:Qt::KeepAspectRatioByExpanding);
            target=QRect(target.x()+(target.width()-size.width())/2,target.y()+(target.height()-size.height())/2,size.width(),size.height());
        }
        p.drawImage(target,image);
    }
    p.restore();
}
}
