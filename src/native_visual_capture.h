// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "main_window.h"
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
namespace LinWinCapture {
inline int run(QApplication &app,const QString &directory) {
    // Never collect a normal user's desktop, apps or documents.
    if(qEnvironmentVariable("LINWIN_ISOLATED_VISUAL_QA")!="1"||!qEnvironmentVariable("XDG_CONFIG_HOME").startsWith("/tmp/linwin-qa-")
       ||!qEnvironmentVariable("XDG_DATA_HOME").startsWith("/tmp/linwin-qa-")) {
        qCritical("Capture requires a disposable build profile.");return 2;
    }
    QDir out(directory);if(!out.exists()&&!QDir().mkpath(directory))return 2;
    QSettings prefs;prefs.setValue("appearance/theme","neon");prefs.setValue("launcher/showRecent",false);prefs.setValue("visual/widgets","show");
    MainWindow window(true,false);window.showFullScreen();
    QTimer::singleShot(1000,&window,[&]{
        auto *panel=window.findChild<QFrame *>("taskbar");
        auto *start=window.findChild<QPushButton *>("startButton");
        auto *menu=window.findChild<QWidget *>("startMenuPanel");
        if(!panel||!start||!menu||!start->isVisible()){app.exit(3);return;}
        const auto desktop=window.grab().toImage();
        if(desktop.isNull()||!desktop.save(out.filePath("desktop.png"))){app.exit(4);return;}
        start->click();
        QTimer::singleShot(250,&window,[&,panel,start,menu]{
            auto *grid=menu->findChild<QListWidget *>("pinnedApps");
            const auto full=window.grab().toImage(),popup=menu->grab().toImage();
            if(!grid||!menu->isVisible()||!start->isChecked()||full.isNull()||popup.isNull()){app.exit(5);return;}
            int violations=0,controls=0;
            QJsonArray bounds;
            if(auto *layout=panel->layout())for(int i=0;i<layout->count();++i){
                auto *w=layout->itemAt(i)->widget();if(!w||!w->isVisible())continue;
                ++controls;const auto r=w->geometry();
                if(!panel->rect().contains(r))++violations;
                bounds.append(QJsonObject{{"name",w->objectName()},{"x",r.x()},{"y",r.y()},{"w",r.width()},{"h",r.height()}});
            }
            int visibleTiles=0,clippedTiles=0;
            for(int i=0;i<grid->count();++i){
                auto *item=grid->item(i);if(item->isHidden())continue;
                const QRect r=grid->visualItemRect(item);
                // Ignore tiles beyond the scroll viewport; examine visible full cells only.
                if(!grid->viewport()->rect().contains(r.center()))continue;
                ++visibleTiles;
                if(r.width()<grid->gridSize().width()-6||r.height()<grid->gridSize().height()-6)++clippedTiles;
            }
            const bool popupFits=window.rect().contains(QRect(menu->mapTo(&window,QPoint()),menu->size()));
            QJsonObject report{{"width",full.width()},{"height",full.height()},{"platform",QApplication::platformName()},
                {"start_width",popup.width()},{"start_height",popup.height()},{"taskbar_controls",controls},
                {"taskbar_out_of_bounds",violations},{"visible_tiles",visibleTiles},{"undersized_tiles",clippedTiles},
                {"start_fits",popupFits},{"taskbar_bounds",bounds}};
            QFile file(out.filePath("layout.json"));if(!file.open(QIODevice::WriteOnly)){app.exit(6);return;}
            file.write(QJsonDocument(report).toJson());file.close();
            const bool saved=full.save(out.filePath("start.png"))&&popup.save(out.filePath("start-detail.png"))
                &&desktop.scaled(768,432,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(out.filePath("desktop-review.jpg"),"JPEG",75)
                &&popup.scaled(600,600,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(out.filePath("start-review.jpg"),"JPEG",84);
            app.exit(saved&&popupFits&&controls>0&&violations==0&&clippedTiles==0?0:7);
        });
    });
    QTimer::singleShot(20000,&app,[&]{app.exit(8);});
    return app.exec();
}
}
