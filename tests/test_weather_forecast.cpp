// SPDX-License-Identifier: GPL-3.0-or-later
#include "weather_forecast.h"
#include <iostream>
int main(){
    const QByteArray sample=R"({"current":{"temperature_2m":21.7,"weather_code":2},"daily":{"time":["2026-11-27","2026-11-28","2026-11-29"],"temperature_2m_max":[24,25,26],"temperature_2m_min":[17,18,19],"weather_code":[2,0,61]}})";
    auto f=LinWinWeather::parse(sample);
    if(!f||f->current!=22||f->code!=2||f->days.size()!=3||f->days[2].high!=26)return 1;
    if(LinWinWeather::parse(QByteArray("{}"))||LinWinWeather::parse(QByteArray(65537,'x'))||LinWinWeather::parse(QByteArray("{")))return 2;
    auto invalid=sample;invalid.replace("21.7","null");if(LinWinWeather::parse(invalid))return 3;
    invalid=sample;invalid.replace("2026-11-28","2026-12-01");if(LinWinWeather::parse(invalid))return 4;
    std::cout<<"PASS: weather forecast validation, malformed and missing data rejected\n";
}
