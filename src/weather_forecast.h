// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QDate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <optional>
#include <cmath>
namespace LinWinWeather {
struct Day { QDate date; int high,low,code; };
struct Forecast { int current,code; QList<Day> days; };
inline bool temperature(const QJsonValue &value,int &out) {
    if(!value.isDouble())return false;
    const double n=value.toDouble();
    if(!std::isfinite(n)||n< -100||n>70)return false;
    out=qRound(n);return true;
}
inline bool code(const QJsonValue &value,int &out) {
    if(!value.isDouble())return false;
    const int n=value.toInt(-1);
    if(n<0||n>99||value.toDouble()!=n)return false;
    out=n;return true;
}
inline std::optional<Forecast> parse(const QByteArray &body) {
    if(body.isEmpty()||body.size()>65536)return {};
    QJsonParseError error;const auto doc=QJsonDocument::fromJson(body,&error);
    if(error.error!=QJsonParseError::NoError||!doc.isObject())return {};
    const auto object=doc.object();
    if(object.value("error").toBool())return {};
    const auto current=object.value("current").toObject(),daily=object.value("daily").toObject();
    Forecast f{};
    if(!temperature(current.value("temperature_2m"),f.current)||!code(current.value("weather_code"),f.code))return {};
    const auto dates=daily.value("time").toArray(),high=daily.value("temperature_2m_max").toArray(),low=daily.value("temperature_2m_min").toArray(),codes=daily.value("weather_code").toArray();
    if(dates.size()!=3||high.size()!=3||low.size()!=3||codes.size()!=3)return {};
    for(int i=0;i<3;++i) {
        Day day{QDate::fromString(dates[i].toString(),Qt::ISODate),0,0,0};
        if(!day.date.isValid()||!temperature(high[i],day.high)||!temperature(low[i],day.low)||day.high<day.low||!code(codes[i],day.code))return {};
        if(i>0&&day.date!=f.days.last().date.addDays(1))return {};
        f.days.append(day);
    }
    return f;
}
}
