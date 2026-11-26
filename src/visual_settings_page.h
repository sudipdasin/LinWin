// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QColorDialog>
#include <QList>
#include <QSettings>
#include <QSignalBlocker>
#include <QScrollArea>
#include <functional>
#include <utility>
class VisualSettingsPage final:public QWidget {
public:
    explicit VisualSettingsPage(QWidget *parent,QString section,std::function<void()> change,std::function<void()> back)
        :QWidget(parent),section_(std::move(section)),change_(std::move(change)){
        setObjectName("linwinVisual_"+section_);
        auto *outer=new QVBoxLayout(this);outer->setContentsMargins(22,18,22,18);
        auto *nav=new QPushButton(tr("Back to Settings"),this);outer->addWidget(nav,0,Qt::AlignLeft);
        connect(nav,&QPushButton::clicked,this,[back]{back();});
        auto *title=new QLabel(tr("Personalization > %1").arg(section_.left(1).toUpper()+section_.mid(1)),this);title->setObjectName("pageTitle");outer->addWidget(title);
        auto *scroll=new QScrollArea(this);scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);outer->addWidget(scroll,1);
        auto *body=new QWidget(scroll);scroll->setWidget(body);layout_=new QVBoxLayout(body);layout_->setSpacing(14);
        auto *note=new QLabel(tr("These controls apply to LinWin. Your Linux desktop and other applications retain their own settings."),body);note->setWordWrap(true);layout_->addWidget(note);
        if(section_=="colors"){
            layout_->addWidget(new QLabel(tr("Choose your shell color mode"),body));
            mode_=new QComboBox(body);mode_->setObjectName("linwinShellColorMode");
            mode_->addItem(tr("Theme default"),"theme");mode_->addItem(tr("Light"),"light");mode_->addItem(tr("Dark"),"dark");layout_->addWidget(mode_);
            connect(mode_,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{QSettings().setValue("visual/shellMode",mode_->currentData());change_();});
            accent_=new QPushButton(body);accent_->setObjectName("linwinAccentColor");layout_->addWidget(accent_);
            connect(accent_,&QPushButton::clicked,this,[this]{const QColor initial(QSettings().value("visual/accent","#29dfff").toString());const auto color=QColorDialog::getColor(initial,this,tr("Choose an accent color"));if(color.isValid()){QSettings().setValue("visual/accent",color.name());reload();change_();}});
            auto *resetAccent=new QPushButton(tr("Use theme accent color"),body);layout_->addWidget(resetAccent);
            connect(resetAccent,&QPushButton::clicked,this,[this]{QSettings().remove("visual/accent");reload();change_();});
            toggle("visual/transparency",tr("Transparency effects (theme-default Start wallpaper blur)"),true);
            disabled(tr("Automatically pick an accent color from my background"));
            disabled(tr("Separate application color mode"));
            disabled(tr("Show accent on native title bars and window borders"));
        }else if(section_=="start"){
            toggle("visual/startFullscreen",tr("Use Start full screen"),false);
            disabled(tr("Show recently added apps"));
            disabled(tr("Show most used apps"));
            disabled(tr("Choose folders that appear on Start"));
        }else if(section_=="taskbar"){
            toggle("visual/smallTaskbar",tr("Use small taskbar buttons"),false);
            toggle("visual/compactAllThemes",tr("Use compact icon taskbar across themes (wide screens, except Classic Minimal)"),true);
            toggle("visual/windowLabels",tr("Show running window labels in compact mode"),false);
            toggle("visual/taskbarSearch",tr("Show search on wide taskbar"),true);
            toggle("visual/taskbarStatus",tr("Show status controls in compact mode"),true);
            toggle("visual/taskbarClock",tr("Show clock"),true);
            disabled(tr("Automatically hide the taskbar"));
            disabled(tr("Taskbar location on screen"));
            disabled(tr("Combine buttons for the same application"));
            disabled(tr("Show badges and notification area icons"));
            disabled(tr("Show taskbar on all displays"));
        }
        auto *reset=new QPushButton(tr("Restore defaults for this page"),body);layout_->addWidget(reset,0,Qt::AlignLeft);
        connect(reset,&QPushButton::clicked,this,[this]{QSettings s;for(const auto &t:toggles_)s.remove(t.key);if(mode_){s.remove("visual/shellMode");s.remove("visual/accent");}s.sync();reload();change_();});
        layout_->addStretch();reload();
    }
    void reload(){
        QSettings s;
        for(const auto &t:toggles_){const QSignalBlocker block(t.box);t.box->setChecked(s.value(t.key,t.initial).toBool());}
        if(mode_){const QSignalBlocker block(mode_);mode_->setCurrentIndex(qMax(0,mode_->findData(s.value("visual/shellMode","theme"))));}
        if(accent_){const QColor color(s.value("visual/accent").toString());accent_->setText(color.isValid()?tr("Accent color: %1").arg(color.name()):tr("Choose custom accent color"));}
    }
private:
    struct Toggle{QString key;QCheckBox *box;bool initial;};
    void toggle(const QString &key,const QString &label,bool initial){
        auto *box=new QCheckBox(label,this);box->setObjectName(key.mid(7));layout_->addWidget(box);toggles_.append({key,box,initial});
        connect(box,&QCheckBox::toggled,this,[this,key](bool value){QSettings().setValue(key,value);change_();});
    }
    void disabled(const QString &label){auto *box=new QCheckBox(label+tr(" — not implemented"),this);box->setEnabled(false);layout_->addWidget(box);}
    QString section_;std::function<void()> change_;QVBoxLayout *layout_=nullptr;QList<Toggle> toggles_;QComboBox *mode_=nullptr;QPushButton *accent_=nullptr;
};
