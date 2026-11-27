// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "theme_visuals.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QFontDatabase>
#include <QFont>
#include <QSettings>
#include <QSignalBlocker>
#include <functional>
#include <utility>
namespace LinWinFont {
inline bool safeFamily(const QString &name) {
    if(name.isEmpty()||name.size()>160)return false;
    for(const auto ch:name)if(!ch.isLetterOrNumber()&&!QStringLiteral(" -_.()").contains(ch))return false;
    return true;
}
inline QString family() {
    const QString value=QSettings().value("visual/fontFamily").toString();
    return safeFamily(value)&&QFontDatabase::families().contains(value)?value:QStringLiteral("Sans Serif");
}
inline QString declaration(const QString &name) {
    return QStringLiteral("font-family: '%1';").arg(safeFamily(name)?name:QStringLiteral("Sans Serif"));
}
}
class AppearanceLibraryPage final:public QWidget {
public:
    explicit AppearanceLibraryPage(QWidget *parent,bool fonts,std::function<void(QString)> apply,std::function<void()> back)
        :QWidget(parent),fonts_(fonts),apply_(std::move(apply)){
        setObjectName(fonts?"linwinFontLibrary":"linwinThemeLibrary");
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(22,18,22,18);layout->setSpacing(10);
        auto *nav=new QPushButton(tr("Back to Settings"),this);layout->addWidget(nav,0,Qt::AlignLeft);connect(nav,&QPushButton::clicked,this,[back]{back();});
        auto *title=new QLabel(fonts?tr("Personalization > Fonts"):tr("Personalization > Themes"),this);title->setObjectName("pageTitle");layout->addWidget(title);
        auto *note=new QLabel(fonts?tr("Choose an installed typeface for LinWin. Other applications and the host desktop are unchanged."):tr("Choose a LinWin theme. Your personal wallpaper and explicit color preferences remain independent."),this);note->setWordWrap(true);layout->addWidget(note);
        search_=new QLineEdit(this);search_->setObjectName(fonts?"linwinFontSearch":"linwinThemeSearch");search_->setPlaceholderText(fonts?tr("Search installed fonts"):tr("Find a theme"));search_->setClearButtonEnabled(true);layout->addWidget(search_);
        list_=new QListWidget(this);list_->setObjectName(fonts?"linwinFontList":"linwinThemeList");list_->setIconSize(QSize(64,40));list_->setAccessibleName(fonts?tr("Installed font families"):tr("Available themes"));layout->addWidget(list_,1);
        preview_=new QLabel(this);preview_->setObjectName(fonts?"linwinFontPreview":"linwinThemePreview");preview_->setTextFormat(Qt::PlainText);preview_->setWordWrap(true);preview_->setMinimumHeight(82);layout->addWidget(preview_);
        auto *actions=new QHBoxLayout;
        auto *reset=new QPushButton(fonts?tr("Use system default"):tr("Restore Neon Fluent"),this);actions->addWidget(reset);actions->addStretch();
        applyButton_=new QPushButton(fonts?tr("Apply font"):tr("Apply theme"),this);applyButton_->setObjectName(fonts?"linwinApplyFont":"linwinApplyTheme");actions->addWidget(applyButton_);layout->addLayout(actions);
        connect(reset,&QPushButton::clicked,this,[this]{apply_(fonts_?QString():QStringLiteral("neon"));reload();});
        connect(applyButton_,&QPushButton::clicked,this,[this]{auto *item=list_->currentItem();if(item&&!item->isHidden())apply_(item->data(Qt::UserRole).toString());});
        connect(list_,&QListWidget::currentItemChanged,this,[this]{preview();});
        connect(search_,&QLineEdit::textChanged,this,[this](const QString &q){for(int i=0;i<list_->count();++i)list_->item(i)->setHidden(!list_->item(i)->text().contains(q,Qt::CaseInsensitive));if(list_->currentItem()&&list_->currentItem()->isHidden())list_->setCurrentRow(-1);preview();});
        reload();
    }
    void reload(){
        search_->clear();list_->clear();QSettings s;
        const QString selected=fonts_?s.value("visual/fontFamily").toString():s.value("appearance/theme","neon").toString();
        if(fonts_){
            for(const auto &name:QFontDatabase::families())if(LinWinFont::safeFamily(name)){auto *item=new QListWidgetItem(name,list_);item->setData(Qt::UserRole,name);}
        }else for(const auto &theme:LinWinTheme::catalog){
            QPixmap image(96,56);image.fill(QColor(LinWinVisual::qstr(theme.sky)));QPainter p(&image);
            p.fillRect(0,42,96,14,QColor(LinWinVisual::qstr(theme.panel)));p.fillRect(5,45,10,8,QColor(LinWinVisual::qstr(theme.accent)));p.end();
            auto *item=new QListWidgetItem(QIcon(image),LinWinVisual::qstr(theme.name),list_);item->setData(Qt::UserRole,LinWinVisual::qstr(theme.id));
        }
        for(int i=0;i<list_->count();++i)if(list_->item(i)->data(Qt::UserRole).toString()==selected){list_->setCurrentRow(i);break;}
        preview();
    }
private:
    void preview(){
        auto *item=list_->currentItem();const bool valid=item&&!item->isHidden();applyButton_->setEnabled(valid);
        if(!valid){preview_->setStyleSheet(QString());preview_->setText(tr("Select an item to preview."));return;}
        const QString id=item->data(Qt::UserRole).toString();
        if(fonts_){preview_->setStyleSheet(LinWinFont::declaration(id)+" font-size:22px;");preview_->setText(tr("The quick brown fox jumps over the lazy dog.\n0123456789 — %1").arg(id));}
        else{const auto &theme=LinWinVisual::theme(id);preview_->setStyleSheet(QStringLiteral("background:%1;color:%2;border:2px solid %3;padding:12px;").arg(LinWinVisual::qstr(theme.panel),LinWinVisual::qstr(theme.text),LinWinVisual::qstr(theme.accent)));preview_->setText(tr("%1\nPreview of the theme palette").arg(item->text()));}
    }
    bool fonts_;std::function<void(QString)> apply_;QLineEdit *search_=nullptr;QListWidget *list_=nullptr;QLabel *preview_=nullptr;QPushButton *applyButton_=nullptr;
};
