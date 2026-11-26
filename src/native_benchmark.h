// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "main_window.h"
#include "desktop_surface.h"
#include "desktop_widgets.h"
#include "shell_icons.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QSettings>
#include <QTimer>
#include <algorithm>
#include <iostream>
#include <sys/resource.h>

namespace LinWinBench {
inline double ms(qint64 ns) {return static_cast<double>(ns)/1000000.0;}
inline void settle(int delay) {QEventLoop loop; QTimer::singleShot(delay,&loop,&QEventLoop::quit); loop.exec();}
inline qint64 residentKiB() {
    QFile f(QStringLiteral("/proc/self/status"));
    if(!f.open(QIODevice::ReadOnly)) return -1;
    for(const auto &line:f.readAll().split('\n'))
        if(line.startsWith("VmRSS:")) {bool ok=false;const auto n=line.mid(6).simplified().split(' ').value(0).toLongLong(&ok);return ok?n:-1;}
    return -1;
}
inline double percentile(QList<qint64> values,int index) {
    std::sort(values.begin(),values.end());
    return ms(values.at(index));
}
inline int run(QApplication &app,const QString &theme) {
    if(app.platformName()!=QStringLiteral("offscreen") || (theme!="neon" && theme!="minimal")) {
        std::cerr<<"Benchmark requires QT_QPA_PLATFORM=offscreen and neon|minimal.\n";return 2;
    }
    QTemporaryDir temporary;
    if(!temporary.isValid())return 3;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,temporary.path());
    QSettings settings;
    settings.setValue(QStringLiteral("appearance/theme"),theme);
    settings.setValue(QStringLiteral("appearance/desktopEffects"),true);
    settings.setValue(QStringLiteral("appearance/desktopGpu"),false);
    settings.setValue(QStringLiteral("appearance/independentWindows"),true);
    QElapsedTimer clock;clock.start();
    MainWindow window(true,false);
    window.resize(1280,720);window.show();app.processEvents();
    const double constructShow=ms(clock.nsecsElapsed());
    auto *surface=window.findChild<QWidget *>(QStringLiteral("windowsDesktop"));
    auto *widget=window.findChild<QWidget *>(QStringLiteral("neonWidgets"));
    if(!surface||!widget)return 4;
    auto *desktop=static_cast<DesktopSurface*>(surface);
    auto *metrics=static_cast<DesktopWidgets*>(widget);
    settle(250);
    const qint64 cachedBytes=desktop->wallpaperCacheBytes();
    const auto before=desktop->rebuildCount();
    const auto rss=residentKiB();
    const bool polling=metrics->pollingActive();
    // Capture actual widget paint, including the desktop's children.
    QList<qint64> frames;
    for(int sample=0;sample<21;++sample) {
        clock.restart();const auto frame=desktop->grab();frames.append(clock.nsecsElapsed());
        if(frame.isNull())return 5;
    }
    const auto after=desktop->rebuildCount();
    const bool minimal=theme==QStringLiteral("minimal");
    if(before!=after || (minimal?(cachedBytes!=0||polling||before!=0):(cachedBytes<=0||!polling||before==0)) || cachedBytes>2560LL*1440*4) return 6;
    auto *cache=LinWinVisual::shellIconCache();
    if(!cache)return 7;
    // Warm all eight glyphs once; compare the original draw path against
    // cached lookup on identical glyph requests.
    for(int glyph=0;glyph<8;++glyph) LinWinVisual::shellIcon(static_cast<LinWinVisual::Glyph>(glyph));
    const auto cachedRasterizations=cache->rasterizations;
    constexpr int requests=800;
    qint64 guard=0;
    clock.restart();
    for(int n=0;n<requests;++n) {
        const auto icon=LinWinVisual::rasterizeShellIcon(static_cast<LinWinVisual::Glyph>(n%8));
        if(icon.isNull())return 8; guard+=icon.availableSizes().size();
    }
    const double uncachedMs=ms(clock.nsecsElapsed());
    clock.restart();
    for(int n=0;n<requests;++n) {
        const auto icon=LinWinVisual::shellIcon(static_cast<LinWinVisual::Glyph>(n%8));
        if(icon.isNull())return 9; guard+=icon.availableSizes().size();
    }
    const double cachedMs=ms(clock.nsecsElapsed());
    if(guard!=requests*8 || cachedRasterizations!=8 || cache->rasterizations!=cachedRasterizations || rss<=0 || cachedMs<=0)return 10;
    rusage usage{};if(getrusage(RUSAGE_SELF,&usage)!=0)return 11;
    const QJsonObject result{
        {"theme",theme},{"platform",app.platformName()},{"qt_version",QString::fromLatin1(qVersion())},
        {"logical_width",desktop->width()},{"logical_height",desktop->height()},
        {"construct_show_ms",constructShow},{"resident_kib_after_settle",static_cast<double>(rss)},
        {"peak_rss_kib_including_benchmark",static_cast<double>(usage.ru_maxrss)},
        {"wallpaper_cache_bytes",static_cast<double>(cachedBytes)},
        {"wallpaper_rebuilds_after_settle",static_cast<double>(before)},
        {"wallpaper_rebuilds_during_21_captures",static_cast<double>(after-before)},
        {"desktop_widget_polling_active",polling},
        {"capture_median_ms",percentile(frames,10)},{"capture_p95_ms",percentile(frames,19)},
        {"icon_requests",requests},{"icon_uncached_ms",uncachedMs},{"icon_cached_ms",cachedMs},
        {"icon_warm_rasterizations",static_cast<int>(cachedRasterizations)},{"icon_additional_cached_rasterizations",static_cast<int>(cache->rasterizations-cachedRasterizations)},
        {"icon_uncached_pixmap_rasterizations",requests*4}
    };
    std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';
    window.close();return 0;
}
}
