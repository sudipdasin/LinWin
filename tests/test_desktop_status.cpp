// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop_status.h"
#include <QTemporaryDir>
#include <QCoreApplication>
#include <iostream>
int main(int argc,char **argv){
    QCoreApplication app(argc,argv);QTemporaryDir tmp;if(!tmp.isValid())return 1;
    auto put=[&](const QString &name,const QString &key,const QByteArray &value){QDir(tmp.path()).mkpath(name);QFile f(tmp.filePath(name+"/"+key));return f.open(QIODevice::WriteOnly|QIODevice::Truncate)&&f.write(value)==value.size();};
    if(LinWinStatus::battery(tmp.path()))return 2;
    if(!put("BAT0","type","Battery")||!put("BAT0","capacity","63\n")||!put("BAT0","status","Charging"))return 3;
    auto b=LinWinStatus::battery(tmp.path());if(!b||b->percent!=63||b->state!="Charging")return 4;
    if(!put("BAT1","type","Battery")||!put("BAT1","capacity","37")||!put("BAT1","status","Discharging"))return 5;
    b=LinWinStatus::battery(tmp.path());if(!b||b->percent!=50)return 6;
    if(!put("BAT0","capacity","101")||!put("BAT1","present","0")||LinWinStatus::battery(tmp.path()))return 7;
    std::cout<<"PASS: battery discovery, capacity validation, aggregation and missing status\n";
}
