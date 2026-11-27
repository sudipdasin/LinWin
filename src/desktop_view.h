// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QListWidget>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QActionGroup>
#include <functional>
namespace LinWinDesktopView {
inline void apply(QListWidget *icons) {
    QSettings s;const int size=s.value("desktop/iconSize",44).toInt();
    const int side=size==32?32:size==64?64:44;
    icons->setIconSize(QSize(side,side));
    icons->setGridSize(QSize(side==32?96:side==64?144:112,side==32?92:side==64?136:110));
    const bool visible=s.value("desktop/showIcons",true).toBool();
    for(int i=0;i<icons->count();++i){
        auto *item=icons->item(i);const QString place=item->data(Qt::UserRole+2).toString();
        const bool show=visible&&(place.isEmpty()||s.value("desktop/place/"+place,true).toBool());
        if(!show&&item->isSelected())item->setSelected(false);
        item->setHidden(!show);
    }
    icons->doItemsLayout();
}
inline QDir::SortFlags sort(QSettings &s) {
    const QString mode=s.value("desktop/sort","name").toString();
    QDir::SortFlags flags=QDir::DirsFirst|QDir::IgnoreCase;
    if(mode=="size")flags|=QDir::Size;
    else if(mode=="type")flags|=QDir::Type;
    else if(mode=="modified")flags|=QDir::Time;
    else flags|=QDir::Name;
    return flags;
}
inline QString label(const QFileInfo &file) {
    if(file.suffix().compare("desktop",Qt::CaseInsensitive)!=0||file.size()>1024*1024)return file.fileName();
    QSettings desktop(file.absoluteFilePath(),QSettings::IniFormat);
    const QString name=desktop.value("Desktop Entry/Name").toString().simplified();
    return name.isEmpty()?file.completeBaseName():name.left(160);
}
inline QIcon icon(const QFileInfo &file,const QIcon &fallback) {
    if(file.suffix().compare("desktop",Qt::CaseInsensitive)!=0||file.size()>1024*1024)return fallback;
    QSettings desktop(file.absoluteFilePath(),QSettings::IniFormat);
    const QString name=desktop.value("Desktop Entry/Icon").toString();
    if(name.isEmpty()||name.contains('/')||name.contains('\\'))return fallback;
    return QIcon::fromTheme(name,fallback);
}
inline void menu(QMenu &menu,QListWidget *icons,const std::function<void()> &refresh) {
    auto *view=menu.addMenu(QObject::tr("View"));
    auto *sizes=new QActionGroup(view);sizes->setExclusive(true);
    QSettings s;const int current=s.value("desktop/iconSize",44).toInt();
    for(const auto &entry:QList<QPair<int,QString>>{{64,QObject::tr("Large icons")},{44,QObject::tr("Medium icons")},{32,QObject::tr("Small icons")}}){
        auto *action=view->addAction(entry.second);action->setCheckable(true);sizes->addAction(action);action->setChecked(current==entry.first);
        QObject::connect(action,&QAction::triggered,icons,[icons,entry]{QSettings().setValue("desktop/iconSize",entry.first);apply(icons);});
    }
    view->addSeparator();
    auto *visible=view->addAction(QObject::tr("Show desktop icons"));visible->setCheckable(true);visible->setChecked(s.value("desktop/showIcons",true).toBool());
    QObject::connect(visible,&QAction::toggled,icons,[icons](bool enabled){QSettings().setValue("desktop/showIcons",enabled);apply(icons);});
    auto *sortMenu=menu.addMenu(QObject::tr("Sort by"));auto *sortGroup=new QActionGroup(sortMenu);sortGroup->setExclusive(true);
    for(const auto &entry:QList<QPair<QString,QString>>{{"name",QObject::tr("Name")},{"size",QObject::tr("Size")},{"type",QObject::tr("Item type")},{"modified",QObject::tr("Date modified")}}){
        auto *action=sortMenu->addAction(entry.second);action->setCheckable(true);sortGroup->addAction(action);action->setChecked(s.value("desktop/sort","name").toString()==entry.first);
        QObject::connect(action,&QAction::triggered,icons,[refresh,entry]{QSettings().setValue("desktop/sort",entry.first);refresh();});
    }
}
}
