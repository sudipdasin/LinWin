// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "background_preferences.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QColorDialog>
#include <QPixmap>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <functional>
class BackgroundSettingsPage final:public QWidget {
public:
    explicit BackgroundSettingsPage(QWidget *parent,std::function<void()> changed,std::function<void()> back)
        :QWidget(parent),changed_(std::move(changed)){
        setObjectName("linwinBackgroundSettings");
        auto *outer=new QVBoxLayout(this);outer->setContentsMargins(22,18,22,18);
        auto *header=new QHBoxLayout;
        auto *backButton=new QPushButton(tr("Back to Settings"),this);backButton->setObjectName("linwinBackgroundBack");
        connect(backButton,&QPushButton::clicked,this,[back]{back();});
        header->addWidget(backButton);header->addStretch();outer->addLayout(header);
        auto *title=new QLabel(tr("Personalization > Background"),this);title->setObjectName("pageTitle");title->setWordWrap(true);outer->addWidget(title);
        auto *scroll=new QScrollArea(this);scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);outer->addWidget(scroll,1);
        auto *body=new QWidget(scroll);scroll->setWidget(body);auto *layout=new QVBoxLayout(body);layout->setSpacing(12);
        preview_=new QLabel(body);preview_->setObjectName("linwinBackgroundPreview");preview_->setFixedSize(320,180);
        preview_->setAlignment(Qt::AlignCenter);preview_->setStyleSheet("background:#10243b;color:white;border:1px solid #49647b;border-radius:6px;");
        preview_->setAccessibleName(tr("Background preview"));layout->addWidget(preview_,0,Qt::AlignHCenter);
        auto *form=new QFormLayout;form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);form->setRowWrapPolicy(QFormLayout::WrapLongRows);layout->addLayout(form);
        mode_=new QComboBox(body);mode_->setObjectName("linwinBackgroundMode");
        mode_->addItem(tr("Picture"),"picture");mode_->addItem(tr("Solid color"),"solid");mode_->addItem(tr("Slideshow"),"slideshow");mode_->addItem(tr("Theme default"),"theme");
        form->addRow(tr("Personalize your background"),mode_);
        pictureRow_=new QWidget(body);auto *pictureLayout=new QHBoxLayout(pictureRow_);pictureLayout->setContentsMargins(0,0,0,0);
        picture_=new QLabel(pictureRow_);picture_->setTextFormat(Qt::PlainText);picture_->setWordWrap(true);
        auto *browse=new QPushButton(tr("Browse"),pictureRow_);browse->setObjectName("linwinBackgroundBrowse");pictureLayout->addWidget(picture_,1);pictureLayout->addWidget(browse);
        form->addRow(tr("Choose a picture"),pictureRow_);
        recent_=new QComboBox(body);recent_->setObjectName("linwinBackgroundRecent");form->addRow(tr("Recent pictures"),recent_);
        folderRow_=new QWidget(body);auto *folderLayout=new QHBoxLayout(folderRow_);folderLayout->setContentsMargins(0,0,0,0);
        folder_=new QLabel(folderRow_);folder_->setTextFormat(Qt::PlainText);folder_->setWordWrap(true);
        auto *browseFolder=new QPushButton(tr("Browse folder"),folderRow_);browseFolder->setObjectName("linwinBackgroundFolderBrowse");folderLayout->addWidget(folder_,1);folderLayout->addWidget(browseFolder);
        form->addRow(tr("Choose an album"),folderRow_);
        minutes_=new QSpinBox(body);minutes_->setObjectName("linwinBackgroundInterval");minutes_->setRange(1,1440);minutes_->setSuffix(tr(" min"));form->addRow(tr("Change picture every"),minutes_);
        fit_=new QComboBox(body);fit_->setObjectName("linwinBackgroundFit");
        for(const auto &entry:QList<QPair<QString,QString>>{{tr("Fill"),"fill"},{tr("Fit"),"fit"},{tr("Stretch"),"stretch"},{tr("Tile"),"tile"},{tr("Center"),"center"},{tr("Span"),"span"}})fit_->addItem(entry.first,entry.second);
        form->addRow(tr("Choose a fit"),fit_);
        color_=new QPushButton(body);color_->setObjectName("linwinBackgroundColor");form->addRow(tr("Background color"),color_);
        form_=form;
        auto *note=new QLabel(tr("Your settings apply to LinWin only. Span uses the virtual desktop geometry; LinWin does not create a desktop surface on monitors it does not occupy."),body);note->setWordWrap(true);layout->addWidget(note);layout->addStretch();
        status_=new QLabel(this);status_->setObjectName("linwinBackgroundStatus");status_->setTextFormat(Qt::PlainText);status_->setWordWrap(true);outer->addWidget(status_);
        auto *apply=new QPushButton(tr("Apply background"),this);apply->setObjectName("linwinBackgroundApply");outer->addWidget(apply,0,Qt::AlignRight);
        connect(browse,&QPushButton::clicked,this,[this]{
            const auto path=QFileDialog::getOpenFileName(this,tr("Choose a desktop picture"),prefs_.picture.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::PicturesLocation):QFileInfo(prefs_.picture).absolutePath(),tr("Images (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff);;All files (*)"));
            if(!path.isEmpty())choosePicture(path);
        });
        connect(browseFolder,&QPushButton::clicked,this,[this]{
            const auto path=QFileDialog::getExistingDirectory(this,tr("Choose slideshow folder"),prefs_.folder.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::PicturesLocation):prefs_.folder);
            if(!path.isEmpty()){prefs_.folder=path;refresh();}
        });
        connect(color_,&QPushButton::clicked,this,[this]{
            const QColor selected=QColorDialog::getColor(prefs_.color,this,tr("Choose background color"));
            if(selected.isValid()){prefs_.color=selected;refresh();}
        });
        connect(recent_,qOverload<int>(&QComboBox::activated),this,[this](int index){const auto path=recent_->itemData(index).toString();if(!path.isEmpty())choosePicture(path);});
        connect(mode_,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{prefs_.mode=mode_->currentData().toString();refresh();});
        connect(fit_,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{prefs_.fit=fit_->currentData().toString();refresh();});
        connect(minutes_,qOverload<int>(&QSpinBox::valueChanged),this,[this](int value){prefs_.minutes=value;});
        connect(apply,&QPushButton::clicked,this,[this]{
            QSettings s;
            if(!LinWinBackground::save(s,prefs_)){status_->setText(tr("Cannot apply: choose a readable image or a folder containing valid images."));return;}
            refreshRecent();changed_();status_->setText(tr("Background applied and saved."));
        });
        reload();
    }
    void reload(){
        QSettings s;prefs_=LinWinBackground::load(s);
        const QSignalBlocker a(mode_),b(fit_),c(minutes_);
        mode_->setCurrentIndex(mode_->findData(prefs_.mode));fit_->setCurrentIndex(fit_->findData(prefs_.fit));minutes_->setValue(prefs_.minutes);
        refreshRecent();refresh();status_->clear();
    }
private:
    void choosePicture(const QString &path){
        if(LinWinBackground::readImage(path).isNull()){status_->setText(tr("This picture is unreadable or exceeds the supported size."));return;}
        prefs_.picture=path;refresh();status_->clear();
    }
    void refreshRecent(){
        QSettings s;const QSignalBlocker block(recent_);
        recent_->clear();recent_->addItem(tr("Select a recent picture"),QString());
        for(const auto &path:s.value("background/recent").toStringList())if(QFileInfo(path).isFile())recent_->addItem(QFileInfo(path).fileName(),path);
    }
    void showRow(QWidget *w,bool visible){
        w->setVisible(visible);if(auto *label=form_->labelForField(w))label->setVisible(visible);
    }
    void refresh(){
        showRow(pictureRow_,prefs_.mode=="picture");showRow(recent_,prefs_.mode=="picture");
        showRow(folderRow_,prefs_.mode=="slideshow");showRow(minutes_,prefs_.mode=="slideshow");
        showRow(fit_,prefs_.mode=="picture"||prefs_.mode=="slideshow");showRow(color_,prefs_.mode!="theme");
        picture_->setText(prefs_.picture.isEmpty()?tr("No picture selected"):prefs_.picture);
        folder_->setText(prefs_.folder.isEmpty()?tr("No folder selected"):prefs_.folder);
        color_->setText(prefs_.color.name());
        if(prefs_.mode=="theme"){preview_->setPixmap(QPixmap());preview_->setText(tr("Uses the current theme background"));return;}
        QImage image;
        if(prefs_.mode=="picture")image=LinWinBackground::readImage(prefs_.picture);
        if(prefs_.mode=="slideshow")for(const auto &path:LinWinBackground::slides(prefs_.folder)){image=LinWinBackground::readImage(path);if(!image.isNull())break;}
        QImage canvas(320,180,QImage::Format_RGB32);QPainter painter(&canvas);LinWinBackground::paint(painter,canvas.rect(),image,prefs_);painter.end();
        preview_->setPixmap(QPixmap::fromImage(canvas));
    }
    LinWinBackground::Preferences prefs_;std::function<void()> changed_;
    QFormLayout *form_=nullptr;QComboBox *mode_=nullptr,*fit_=nullptr,*recent_=nullptr;QSpinBox *minutes_=nullptr;
    QLabel *preview_=nullptr,*picture_=nullptr,*folder_=nullptr,*status_=nullptr;
    QWidget *pictureRow_=nullptr,*folderRow_=nullptr;QPushButton *color_=nullptr;
};
