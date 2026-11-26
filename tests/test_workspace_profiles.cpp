// SPDX-License-Identifier: GPL-3.0-or-later
#include "workspace_profiles.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <iostream>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    QTemporaryDir tmp; if(!tmp.isValid()) return 1;
    const auto file=tmp.filePath("workspace.ini");
    QSettings s(file,QSettings::IniFormat);
    using namespace LinWinWorkspace;
    const QList<Placement> pair{{1,QRect(-1400,100,700,500),false},{2,QRect(100,200,800,600),true}};
    if(!save(s,1,pair)||save(s,0,pair)||save(s,2,{})||save(s,2,{pair[0],pair[0]})) return 2;
    QSettings reopened(file,QSettings::IniFormat);
    const auto loaded=load(reopened,1);
    if(loaded.size()!=2||loaded[0].geometry!=pair[0].geometry||!loaded[1].maximized||!load(reopened,2).isEmpty())return 3;
    if(fit(QRect(-1400,100,700,500),QRect(0,0,1024,768))!=QRect(0,100,700,500))return 4;
    if(fit(QRect(-2000,-2000,4000,4000),QRect(-1280,0,1280,720))!=QRect(-1280,0,1280,720))return 5;
    if(save(s,2,{{5,QRect(0,0,400,400),false}})||save(s,2,{{1,QRect(0,0,1,1),false}}))return 6;
    s.setValue(key(3),QVariantList{QVariantMap{{"tool",1},{"geometry",QRect(0,0,400,400)}},QVariantMap{{"tool",1},{"geometry",QRect(0,0,500,500)}}});
    if(!load(s,3).isEmpty())return 7;
    if(!rememberGeometry(s,1,pair[0].geometry)||rememberGeometry(s,5,pair[0].geometry))return 8;
    s.sync(); reopened.sync();
    if(rememberedGeometry(reopened,1)!=pair[0].geometry||rememberedGeometry(reopened,2).isValid())return 9;
    reopened.setValue(QStringLiteral("appearance/rememberToolGeometry"),false);
    if(rememberedGeometry(reopened,1).isValid()||rememberGeometry(reopened,1,pair[1].geometry))return 10;
    std::cout<<"PASS: persistent workspace slots, validation, duplicate rejection, monitor fitting and optional geometry recall\n";
}
