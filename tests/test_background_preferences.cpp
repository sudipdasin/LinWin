// SPDX-License-Identifier: GPL-3.0-or-later
#include "background_preferences.h"
#include <QTemporaryDir>
#include <QCoreApplication>
#include <iostream>
int main(int argc,char **argv){
    QCoreApplication app(argc,argv);QTemporaryDir dir;if(!dir.isValid())return 1;
    QSettings s(dir.filePath("prefs.ini"),QSettings::IniFormat);
    using namespace LinWinBackground;
    auto p=load(s);if(p.mode!="theme"||p.fit!="fill")return 2;
    QImage source(20,10,QImage::Format_RGB32);source.fill(Qt::red);
    const auto file=dir.filePath("sample.png");if(!source.save(file))return 3;
    p.mode="picture";p.picture=file;p.fit="fit";p.color=Qt::blue;
    if(!save(s,p)||load(s).picture!=file||s.value("background/recent").toStringList()!=QStringList{file})return 4;
    auto frame=[&](QString fit){p.fit=fit;QImage result(40,40,QImage::Format_RGB32);QPainter painter(&result);paint(painter,result.rect(),source,p);painter.end();return result;};
    if(frame("fit").pixelColor(0,0)!=Qt::blue||frame("fit").pixelColor(20,20)!=Qt::red)return 5;
    for(const auto &fit:{"fill","stretch","tile","span"})if(frame(fit).pixelColor(0,0)!=Qt::red)return 6;
    if(frame("center").pixelColor(0,0)!=Qt::blue||frame("center").pixelColor(20,20)!=Qt::red)return 7;
    p.picture=dir.filePath("missing.png");if(save(s,p)||load(s).picture!=file)return 8;
    p.mode="slideshow";p.folder=dir.path();p.minutes=1;if(!save(s,p)||slides(dir.path()).size()!=1)return 9;
    p.minutes=0;if(save(s,p))return 10;
    p.minutes=10;p.mode="solid";if(!save(s,p)||load(s).mode!="solid")return 11;
    std::cout<<"PASS: picture, solid, slideshow, fit geometry, missing-image rejection, recent history and persistence\n";
}
