// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QSettings>
#include <QRect>
#include <QList>
#include <QSet>
#include <QVariant>
namespace LinWinWorkspace {
struct Placement { int tool; QRect geometry; bool maximized; };
inline QString key(int slot) { return QStringLiteral("workspace/profiles/v1/%1").arg(slot); }
inline bool validSlot(int slot) { return slot>=1 && slot<=3; }
inline bool valid(const Placement &p) {
    return p.tool>=1 && p.tool<=4 && p.geometry.width()>=100 && p.geometry.height()>=100
        && p.geometry.width()<=32768 && p.geometry.height()<=32768
        && p.geometry.x()>=-100000 && p.geometry.x()<=100000 && p.geometry.y()>=-100000 && p.geometry.y()<=100000;
}
inline bool save(QSettings &s,int slot,const QList<Placement> &placements) {
    if(!validSlot(slot) || placements.isEmpty() || placements.size()>4) return false;
    QSet<int> seen;
    QVariantList records;
    for(const auto &p:placements) {
        if(!valid(p) || seen.contains(p.tool)) return false;
        seen.insert(p.tool);
        records.append(QVariantMap{{QStringLiteral("tool"),p.tool},{QStringLiteral("geometry"),p.geometry},{QStringLiteral("maximized"),p.maximized}});
    }
    s.setValue(key(slot),records); s.sync();
    return s.status()==QSettings::NoError;
}
inline QList<Placement> load(QSettings &s,int slot) {
    if(!validSlot(slot)) return {};
    const auto records=s.value(key(slot)).toList();
    if(records.size()>4) return {};
    QList<Placement> result; QSet<int> seen;
    for(const auto &record:records) {
        const auto map=record.toMap(); bool ok=false;
        const int tool=map.value(QStringLiteral("tool")).toInt(&ok);
        const Placement p{tool,map.value(QStringLiteral("geometry")).toRect(),map.value(QStringLiteral("maximized")).toBool()};
        if(!ok || !valid(p) || seen.contains(tool)) return {};
        seen.insert(tool);result.append(p);
    }
    return result;
}
inline QString geometryKey(int tool) {return QStringLiteral("workspace/toolGeometry/v1/%1").arg(tool);}
inline bool rememberGeometry(QSettings &s,int tool,const QRect &geometry) {
    if(!s.value(QStringLiteral("appearance/rememberToolGeometry"),true).toBool() || !valid({tool,geometry,false})) return false;
    s.setValue(geometryKey(tool),geometry);
    return true;
}
inline QRect rememberedGeometry(QSettings &s,int tool) {
    if(!s.value(QStringLiteral("appearance/rememberToolGeometry"),true).toBool() || tool<1 || tool>4) return {};
    const auto geometry=s.value(geometryKey(tool)).toRect();
    return valid({tool,geometry,false})?geometry:QRect();
}
inline QRect fit(const QRect &r,const QRect &area) {
    if(!r.isValid() || !area.isValid()) return {};
    const QSize size=r.size().boundedTo(area.size());
    return QRect(QPoint(qBound(area.left(),r.x(),area.right()-size.width()+1),qBound(area.top(),r.y(),area.bottom()-size.height()+1)),size);
}
}
