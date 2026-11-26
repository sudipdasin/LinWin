// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "windows10_settings_catalog.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QResizeEvent>
#include <QToolButton>
#include <QSignalBlocker>
#include "shell_icons.h"
#include <functional>
#include <utility>
class WindowsSettingsCatalogPage final:public QWidget {
public:
    explicit WindowsSettingsCatalogPage(QWidget *parent,std::function<void(QString)> open,std::function<void()> legacy):QWidget(parent),open_(std::move(open)){
        setObjectName("linwinWindowsSettingsCatalog");
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(22,18,22,18);
        auto *header=new QHBoxLayout;
        auto *heading=new QLabel(tr("Settings"),this);heading->setObjectName("pageTitle");header->addWidget(heading);header->addStretch();
        auto *home=new QPushButton(tr("Settings home"),this);home->setObjectName("linwinCatalogHome");header->addWidget(home);layout->addLayout(header);
        connect(home,&QPushButton::clicked,this,[this]{category_.clear();search_->clear();populate();});
        auto *note=new QLabel(tr("Windows 10 settings structure. Visual settings first, ease of use second. Unavailable controls remain visible but cannot run."),this);note->setWordWrap(true);layout->addWidget(note);
        search_=new QLineEdit(this);search_->setObjectName("linwinCatalogSearch");search_->setPlaceholderText(tr("Find a setting"));search_->setClearButtonEnabled(true);layout->addWidget(search_);
        home_=new QScrollArea(this);home_->setObjectName("linwinSettingsHome");home_->setWidgetResizable(true);home_->setFrameShape(QFrame::NoFrame);
        auto *cards=new QWidget(home_);home_->setWidget(cards);cardsLayout_=new QGridLayout(cards);cardsLayout_->setContentsMargins(4,4,4,4);cardsLayout_->setSpacing(10);
        QString previous;
        for(const auto &entry:LinWinSettings::catalog){
            const QString category=str(entry.category);if(category==previous)continue;previous=category;
            auto *card=new QToolButton(cards);card->setObjectName("linwinSettingsCategory");card->setText(category);
            card->setAccessibleName(category);card->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            const auto glyph=category=="Personalization"?LinWinVisual::Glyph::Settings:category=="Search"?LinWinVisual::Glyph::Search:category=="Apps"?LinWinVisual::Glyph::Files:LinWinVisual::Glyph::Desktop;
            const QString iconName=category=="Personalization"?"preferences-desktop-theme":category=="System"?"computer":category=="Devices"?"input-keyboard":category=="Network & Internet"?"network-workgroup":category=="Accounts"?"avatar-default":category=="Time & Language"?"preferences-system-time":category=="Apps"?"applications-other":category=="Search"?"system-search":category=="Privacy"?"security-high":category=="Update & Security"?"system-software-update":category=="Ease of Access"?"preferences-desktop-accessibility":category=="Gaming"?"applications-games":"phone";
            card->setIcon(QIcon::fromTheme(iconName,LinWinVisual::shellIcon(glyph)));card->setIconSize(QSize(32,32));
            card->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);card->setMinimumHeight(92);card->setAutoRaise(false);
            int total=0,available=0;for(const auto &row:LinWinSettings::catalog)if(row.category==entry.category){++total;if(!row.route.empty())++available;}
            card->setToolTip(tr("%1 settings · %2 available or partial").arg(total).arg(available));
            connect(card,&QToolButton::clicked,this,[this,category]{category_=category;{const QSignalBlocker guard(search_);search_->clear();}populate();});
            cards_.append(card);
        }
        arrangeCards();layout->addWidget(home_,1);
        tree_=new QTreeWidget(this);tree_->setObjectName("linwinCatalogTree");tree_->setHeaderLabels({tr("Category / setting"),tr("Priority"),tr("Availability")});tree_->setRootIsDecorated(true);tree_->setAlternatingRowColors(true);
        tree_->header()->setSectionResizeMode(0,QHeaderView::Stretch);tree_->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);tree_->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);layout->addWidget(tree_,1);
        detail_=new QLabel(tr("Select a setting."),this);detail_->setWordWrap(true);layout->addWidget(detail_);
        auto *buttons=new QHBoxLayout;auto *existing=new QPushButton(tr("Existing LinWin controls"),this);existing->setObjectName("linwinCatalogLegacy");buttons->addWidget(existing);buttons->addStretch();
        openButton_=new QPushButton(tr("Open setting"),this);openButton_->setObjectName("linwinCatalogOpen");openButton_->setEnabled(false);buttons->addWidget(openButton_);layout->addLayout(buttons);
        connect(existing,&QPushButton::clicked,this,[legacy]{legacy();});
        connect(search_,&QLineEdit::textChanged,this,[this]{category_.clear();populate();});
        connect(tree_,&QTreeWidget::currentItemChanged,this,[this]{updateSelection();});
        connect(openButton_,&QPushButton::clicked,this,[this]{activate();});
        connect(tree_,&QTreeWidget::itemActivated,this,[this](QTreeWidgetItem *item,int){if(item==tree_->currentItem())activate();});
        populate();
    }
    void focusSearch(){category_.clear();search_->clear();populate();search_->setFocus();search_->selectAll();}
protected:
    void resizeEvent(QResizeEvent *event) override {QWidget::resizeEvent(event);arrangeCards();}
private:
    void arrangeCards(){if(!cardsLayout_)return;const int columns=width()>=900?3:width()>=450?2:1;if(columns==columns_)return;columns_=columns;for(int i=0;i<cards_.size();++i){cardsLayout_->removeWidget(cards_[i]);cardsLayout_->addWidget(cards_[i],i/columns,i%columns);}for(int i=0;i<3;++i)cardsLayout_->setColumnStretch(i,i<columns?1:0);}
    static QString str(std::string_view s){return QString::fromUtf8(s.data(),static_cast<qsizetype>(s.size()));}
    void populate(){
        tree_->clear();QTreeWidgetItem *group=nullptr;QString last;
        const auto needle=search_->text().trimmed();
        const bool atHome=needle.isEmpty()&&category_.isEmpty();home_->setVisible(atHome);tree_->setVisible(!atHome);
        for(const auto &row:LinWinSettings::catalog){
            const auto category=str(row.category),name=str(row.page),route=str(row.route);
            if(!category_.isEmpty()&&category!=category_)continue;
            if(!needle.isEmpty()&&!(category+" "+name).contains(needle,Qt::CaseInsensitive))continue;
            if(category!=last){group=new QTreeWidgetItem(tree_,{category});group->setFirstColumnSpanned(true);last=category;}
            const auto priority=row.priority==LinWinSettings::Priority::Visual?tr("1 · Visual"):row.priority==LinWinSettings::Priority::Ease?tr("2 · Ease of use"):tr("Deferred");
            const auto state=route.isEmpty()?(row.priority==LinWinSettings::Priority::Deferred?tr("Disabled · deferred"):tr("Disabled · pending")):route=="background"?tr("Available"):tr("Partial · existing controls");
            auto *item=new QTreeWidgetItem(group,{name,priority,state});item->setData(0,Qt::UserRole,route);
        }
        if(!needle.isEmpty()||!category_.isEmpty())tree_->expandAll();else for(int i=0;i<tree_->topLevelItemCount();++i)if(tree_->topLevelItem(i)->text(0)=="Personalization")tree_->topLevelItem(i)->setExpanded(true);
        updateSelection();
    }
    void updateSelection(){
        auto *item=tree_->currentItem();const auto route=item?item->data(0,Qt::UserRole).toString():QString();
        openButton_->setEnabled(!route.isEmpty());detail_->setText(!item?tr("Select a setting."):!item->parent()?tr("Expand this category to view its settings."):route.isEmpty()?tr("This setting is visible for navigation but its action is disabled."):tr("Open the implemented LinWin controls for this setting."));
    }
    void activate(){auto *item=tree_->currentItem();if(!item)return;const auto route=item->data(0,Qt::UserRole).toString();if(!route.isEmpty())open_(route);}
    QString category_;QScrollArea *home_=nullptr;QGridLayout *cardsLayout_=nullptr;QList<QToolButton *> cards_;int columns_=0;
    QLineEdit *search_=nullptr;QTreeWidget *tree_=nullptr;QLabel *detail_=nullptr;QPushButton *openButton_=nullptr;std::function<void(QString)> open_;
};
