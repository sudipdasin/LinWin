// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "main_window.h"
#include "settings_hub_widget.h"
#include "x11_dock.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QListWidget>
#include <QFrame>
#include <QTimer>
#include <QSettings>
#include <QScreen>
#include <QTextStream>
#include <QLayout>
#include <QProcess>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QAction>
#include <QRegularExpression>
#include <functional>
namespace LinWinCapture {
inline QString command(const QString &program,const QStringList &args){
    QProcess process;process.start(program,args);
    if(!process.waitForStarted(1000)||!process.waitForFinished(1500)||process.exitCode()!=0)return {};
    return QString::fromLocal8Bit(process.readAllStandardOutput());
}
inline bool until(const std::function<bool()> &condition,int ms=6500){
    QElapsedTimer elapsed;elapsed.start();
    do{if(condition())return true;QEventLoop loop;QTimer::singleShot(80,&loop,&QEventLoop::quit);loop.exec();}
    while(elapsed.elapsed()<ms);return false;
}
inline QPushButton *externalButton(MainWindow &window,const QString &title){
    for(auto *button:window.findChildren<QPushButton *>("linwinExternalWindow"))if(button->toolTip()==title)return button;
    return nullptr;
}
inline bool verifyWindowActions(MainWindow &window,QJsonObject &report){
    // Only invoked in the isolated QA X11 session containing these exact
    // test terminals. Never enumerate or close a customer's windows.
    const QString titleA="LinWin QA Window A",titleB="LinWin QA Window B";
    const bool listed=until([&]{return externalButton(window,titleA)&&externalButton(window,titleB);});
    report["external_windows_discovered"]=listed;if(!listed)return false;
    auto *a=externalButton(window,titleA);auto *b=externalButton(window,titleB);
    const QString idA=a->property("linwinWindowId").toString(),idB=b->property("linwinWindowId").toString();
    bool okA=false,okB=false;
    const auto valueA=idA.toULongLong(&okA,16),valueB=idB.toULongLong(&okB,16);
    if(!okA||!okB||!valueA||!valueB||valueA==valueB)return false;
    a->click();
    const bool activated=until([&]{
        const auto match=QRegularExpression("0x[0-9a-fA-F]+").match(command("xprop",{"-root","_NET_ACTIVE_WINDOW"}));
        bool valid=false;const auto actual=match.captured().toULongLong(&valid,16);
        return valid&&actual==valueA;
    });
    report["external_window_activated"]=activated;if(!activated)return false;
    const bool indicated=until([&]{auto *first=externalButton(window,titleA);auto *second=externalButton(window,titleB);return first&&second&&first->property("linwinActiveWindow").toBool()&&!second->property("linwinActiveWindow").toBool();});
    report["active_window_indicated"]=indicated;if(!indicated)return false;
    b=externalButton(window,titleB);if(!b)return false;
    auto *close=b->findChild<QAction *>("linwinCloseExternalWindow");if(!close)return false;
    close->trigger();
    const bool closed=until([&]{
        const auto list=command("wmctrl",{"-l"});
        return !list.isEmpty()&&!list.contains(titleB)&&list.contains(titleA)&&!externalButton(window,titleB);
    });
    report["external_window_closed"]=closed;return closed;
}
inline bool workareaHeight(int expected){
    const QString raw=command("xprop",{"-root","_NET_WORKAREA"});
    const int eq=raw.indexOf('=');if(eq<0)return false;
    const auto values=raw.mid(eq+1).split(',',Qt::SkipEmptyParts);
    bool ok=false;return values.size()>=4&&values.at(3).trimmed().toInt(&ok)==expected&&ok;
}
inline bool verifyDock(MainWindow &window,QFrame *panel,QJsonObject &report,const QRect &screen){
    const bool detached=panel->isWindow()&&panel->isVisible()&&panel->property("linwinDetachedDock").toBool();
    report["independent_dock"]=detached;if(!detached)return false;
    const auto dockId=static_cast<xcb_window_t>(panel->winId());
    LinWinDock::Connection connection;
    std::array<uint32_t,12> strut{};
    const bool reserved=until([&]{return connection.read(dockId,"_NET_WM_STRUT_PARTIAL",XCB_ATOM_CARDINAL,strut.data(),12)&&strut[3]==static_cast<uint32_t>(panel->height())&&workareaHeight(screen.height()-panel->height());});
    report["workarea_reserved"]=reserved;if(!reserved)return false;
    auto *a=externalButton(window,"LinWin QA Window A");if(!a)return false;
    const QString id=a->property("linwinWindowId").toString();
    command("wmctrl",{"-ir",id,"-b","add,maximized_vert,maximized_horz"});
    a->click();
    const bool usable=until([&]{
        const QString state=command("xprop",{"-id",id,"_NET_WM_STATE"});
        if(!state.contains("_NET_WM_STATE_MAXIMIZED_VERT")||!state.contains("_NET_WM_STATE_MAXIMIZED_HORZ"))return false;
        const QString info=command("xwininfo",{"-id",id});
        const auto y=QRegularExpression("Absolute upper-left Y:\\s*(-?\\d+)").match(info);
        const auto height=QRegularExpression("Height:\\s*(\\d+)").match(info);
        const auto extents=command("xprop",{"-id",id,"_NET_FRAME_EXTENTS"}).section('=',1).split(',');
        if(!y.hasMatch()||!height.hasMatch()||extents.size()!=4)return false;
        bool ok=false;const int bottom=extents.last().trimmed().toInt(&ok);
        if(!ok||y.captured(1).toInt()+height.captured(1).toInt()+bottom>panel->geometry().top()+2)return false;
        const QString stack=command("xprop",{"-root","_NET_CLIENT_LIST_STACKING"});
        auto matches=QRegularExpression("0x[0-9a-fA-F]+").globalMatch(stack);
        int index=0,appIndex=-1,dockIndex=-1;
        bool parsed=false;const auto appId=id.toULongLong(&parsed,16);if(!parsed)return false;
        while(matches.hasNext()){const auto value=matches.next().captured().toULongLong(nullptr,16);if(value==appId)appIndex=index;if(value==dockId)dockIndex=index;++index;}
        return panel->isVisible()&&appIndex>=0&&dockIndex>appIndex;
    });
    report["maximized_window_respects_dock"]=usable;
    return usable;
}
inline int run(QApplication &app,const QString &directory) {
    // Never collect a normal user's desktop, apps or documents.
    if(qEnvironmentVariable("LINWIN_ISOLATED_VISUAL_QA")!="1"||!qEnvironmentVariable("XDG_CONFIG_HOME").startsWith("/tmp/linwin-qa-")
       ||!qEnvironmentVariable("XDG_DATA_HOME").startsWith("/tmp/linwin-qa-")) {
        qCritical("Capture requires a disposable build profile.");return 2;
    }
    QDir out(directory);if(!out.exists()&&!QDir().mkpath(directory))return 2;
    QSettings prefs;prefs.setValue("appearance/theme","neon");prefs.setValue("launcher/showRecent",false);prefs.setValue("visual/widgets","show");
    // Xvfb has no window manager to honor EWMH fullscreen requests.
    // Size the frameless overlay explicitly to the real X11 screen geometry.
    const auto *screen=app.primaryScreen();
    if(!screen||!screen->geometry().isValid()){qCritical("No valid X11 screen.");return 2;}
    const QRect screenGeometry=screen->geometry();
    const bool windowActions=qEnvironmentVariable("LINWIN_QA_WINDOW_ACTIONS")=="1";
    const bool dockMode=qEnvironmentVariable("LINWIN_QA_DOCK")=="1";
    prefs.setValue("visual/detachedTaskbar",dockMode);
    prefs.setValue("visual/taskbarPosition","bottom");
    MainWindow window(true,false);
    window.setGeometry(screenGeometry);window.show();
    qInfo("Capture requested screen: %dx%d",screenGeometry.width(),screenGeometry.height());
    QTimer::singleShot(1000,&window,[&]{
        if(window.size()!=screenGeometry.size()){
            qCritical("Native overlay size mismatch: actual %dx%d, expected %dx%d",window.width(),window.height(),screenGeometry.width(),screenGeometry.height());
            app.exit(9);return;
        }
        auto *panel=window.findChild<QFrame *>("taskbar");
        auto *start=window.findChild<QPushButton *>("startButton");
        auto *menu=window.findChild<QWidget *>("startMenuPanel");
        if(!panel||!start||!menu||!start->isVisible()){app.exit(3);return;}
        if(windowActions&&!until([&]{return externalButton(window,"LinWin QA Window A")&&externalButton(window,"LinWin QA Window B");})){qCritical("Real QA windows not discovered.");app.exit(10);return;}
        QJsonObject dockReport;
        if(dockMode&&!verifyDock(window,panel,dockReport,screenGeometry)){qCritical("Independent dock acceptance failed.");QFile failed(out.filePath("layout.json"));if(failed.open(QIODevice::WriteOnly))failed.write(QJsonDocument(dockReport).toJson());app.exit(11);return;}
        const auto desktop=dockMode?screen->grabWindow(0).toImage():window.grab().toImage();
        if(desktop.isNull()||!desktop.save(out.filePath("desktop.png"))){app.exit(4);return;}
        start->click();
        QTimer::singleShot(250,&window,[&,panel,start,menu,desktop,dockReport]{
            auto *grid=menu->findChild<QListWidget *>("pinnedApps");
            const auto full=dockMode?screen->grabWindow(0).toImage():window.grab().toImage(),popup=menu->grab().toImage();
            if(!grid||!menu->isVisible()||!start->isChecked()||full.isNull()||popup.isNull()){app.exit(5);return;}
            int violations=0,controls=0;
            QJsonArray bounds;
            if(auto *layout=panel->layout())for(int i=0;i<layout->count();++i){
                auto *w=layout->itemAt(i)->widget();if(!w||!w->isVisible())continue;
                ++controls;const auto r=w->geometry();
                if(!panel->rect().contains(r))++violations;
                bounds.append(QJsonObject{{"name",w->objectName()},{"x",r.x()},{"y",r.y()},{"w",r.width()},{"h",r.height()}});
            }
            int visibleTiles=0,clippedTiles=0,completeTiles=0,firstRowY=-1,firstRowTiles=0;
            QJsonArray tileBounds;
            for(int i=0;i<grid->count();++i){
                auto *item=grid->item(i);if(item->isHidden())continue;
                const QRect r=grid->visualItemRect(item);
                // Ignore tiles beyond the scroll viewport; examine visible full cells only.
                if(!grid->viewport()->rect().contains(r.center()))continue;
                ++visibleTiles;
                if(grid->viewport()->rect().contains(r)){
                    ++completeTiles;if(firstRowY<0)firstRowY=r.top();if(r.top()==firstRowY)++firstRowTiles;
                    tileBounds.append(QJsonObject{{"x",r.x()},{"y",r.y()},{"w",r.width()},{"h",r.height()}});
                }
                if(r.width()<grid->gridSize().width()-6||r.height()<grid->gridSize().height()-6)++clippedTiles;
            }
            const bool popupFits=window.rect().contains(QRect(window.mapFromGlobal(menu->mapToGlobal(QPoint())),menu->size()));
            QJsonObject report{{"width",full.width()},{"height",full.height()},{"platform",QApplication::platformName()},
                {"start_width",popup.width()},{"start_height",popup.height()},{"taskbar_controls",controls},
                {"taskbar_out_of_bounds",violations},{"visible_tiles",visibleTiles},{"complete_tiles",completeTiles},{"first_row_tiles",firstRowTiles},{"tile_bounds",tileBounds},{"registered_pins",grid->count()},{"undersized_tiles",clippedTiles},
                {"start_fits",popupFits},{"taskbar_bounds",bounds}};

            const bool saved=full.save(out.filePath("start.png"))&&popup.save(out.filePath("start-detail.png"))
                &&desktop.scaled(768,432,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(out.filePath("desktop-review.jpg"),"JPEG",75)
                &&popup.scaled(720,720,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(out.filePath("start-review.jpg"),"JPEG",85)
                &&panel->grab().toImage().scaledToWidth(1440,Qt::SmoothTransformation).save(out.filePath("taskbar-review.jpg"),"JPEG",88);
            for(auto it=dockReport.begin();it!=dockReport.end();++it)report[it.key()]=it.value();
            bool actionPass=true;
            if(windowActions){start->setChecked(false);actionPass=verifyWindowActions(window,report);}
            if(dockMode){
                QSettings().setValue("visual/detachedTaskbar",false);
                auto *hub=window.findChild<SettingsHubWidget *>();
                if(hub)hub->visualChanged();
                const bool restored=hub&&until([&]{return !panel->isWindow()&&!panel->property("linwinDetachedDock").toBool()&&workareaHeight(screenGeometry.height());});
                report["dock_restored_and_workarea_released"]=restored;actionPass=actionPass&&restored;
            }
            QFile file(out.filePath("layout.json"));if(!file.open(QIODevice::WriteOnly)){app.exit(6);return;}
            file.write(QJsonDocument(report).toJson());file.close();
            const bool passed=saved&&popupFits&&controls>0&&violations==0&&clippedTiles==0&&completeTiles>=5&&firstRowTiles==4&&actionPass;
            qInfo("Installed capture: saved=%d popupFits=%d controls=%d outOfBounds=%d undersizedTiles=%d completeTiles=%d firstRowTiles=%d",
                  int(saved),int(popupFits),controls,violations,clippedTiles,completeTiles,firstRowTiles);
            app.exit(passed?0:7);
        });
    });
    QTimer::singleShot(dockMode?55000:35000,&app,[&]{app.exit(8);});
    return app.exec();
}
}
