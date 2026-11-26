// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QDir>
#include <QFile>
#include <QList>
#include <QString>
#include <optional>
namespace LinWinStatus {
struct Battery { int percent; QString state; };
inline QString read(const QString &path) {
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly))return {};
    return QString::fromUtf8(f.read(128)).trimmed();
}
inline std::optional<Battery> battery(const QString &root=QStringLiteral("/sys/class/power_supply")) {
    const QDir dir(root);
    int sum=0,count=0;bool charging=false,discharging=false,full=true;
    for(const auto &name:dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        const auto path=dir.filePath(name);
        if(read(path+"/type")!="Battery" || read(path+"/present")=="0")continue;
        bool ok=false;const int capacity=read(path+"/capacity").toInt(&ok);
        if(!ok||capacity<0||capacity>100)continue;
        const auto state=read(path+"/status");
        charging|=state=="Charging";discharging|=state=="Discharging";full&=state=="Full";
        sum+=capacity;++count;
    }
    if(!count)return {};
    return Battery{(sum+count/2)/count,charging?QStringLiteral("Charging"):discharging?QStringLiteral("Discharging"):full?QStringLiteral("Full"):QStringLiteral("Unknown")};
}
}
