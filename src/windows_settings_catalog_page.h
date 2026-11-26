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
#include <functional>
#include <utility>
class WindowsSettingsCatalogPage final:public QWidget {
public:
    explicit WindowsSettingsCatalogPage(QWidget *parent,std::function<void(QString)> open,std::function<void()> legacy):QWidget(parent),open_(std::move(open)){
        setObjectName("linwinWindowsSettingsCatalog");
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(22,18,22,18);
        auto *heading=new QLabel(tr("Settings"),this);heading->setObjectName("pageTitle");layout->addWidget(heading);
        auto *note=new QLabel(tr("Windows 10 settings structure. Visual settings first, ease of use second. Unavailable controls remain visible but cannot run."),this);note->setWordWrap(true);layout->addWidget(note);
        search_=new QLineEdit(this);search_->setObjectName("linwinCatalogSearch");search_->setPlaceholderText(tr("Find a setting"));search_->setClearButtonEnabled(true);layout->addWidget(search_);
        tree_=new QTreeWidget(this);tree_->setObjectName("linwinCatalogTree");tree_->setHeaderLabels({tr("Category / setting"),tr("Priority"),tr("Availability")});tree_->setRootIsDecorated(true);tree_->setAlternatingRowColors(true);
        tree_->header()->setSectionResizeMode(0,QHeaderView::Stretch);tree_->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);tree_->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);layout->addWidget(tree_,1);
        detail_=new QLabel(tr("Select a setting."),this);detail_->setWordWrap(true);layout->addWidget(detail_);
        auto *buttons=new QHBoxLayout;auto *existing=new QPushButton(tr("Existing LinWin controls"),this);existing->setObjectName("linwinCatalogLegacy");buttons->addWidget(existing);buttons->addStretch();
        openButton_=new QPushButton(tr("Open setting"),this);openButton_->setObjectName("linwinCatalogOpen");openButton_->setEnabled(false);buttons->addWidget(openButton_);layout->addLayout(buttons);
        connect(existing,&QPushButton::clicked,this,[legacy]{legacy();});
        connect(search_,&QLineEdit::textChanged,this,[this]{populate();});
        connect(tree_,&QTreeWidget::currentItemChanged,this,[this]{updateSelection();});
        connect(openButton_,&QPushButton::clicked,this,[this]{activate();});
        connect(tree_,&QTreeWidget::itemActivated,this,[this](QTreeWidgetItem *item,int){if(item==tree_->currentItem())activate();});
        populate();
    }
    void focusSearch(){search_->setFocus();search_->selectAll();}
private:
    static QString str(std::string_view s){return QString::fromUtf8(s.data(),static_cast<qsizetype>(s.size()));}
    void populate(){
        tree_->clear();QTreeWidgetItem *group=nullptr;QString last;
        const auto needle=search_->text().trimmed();
        for(const auto &row:LinWinSettings::catalog){
            const auto category=str(row.category),name=str(row.page),route=str(row.route);
            if(!needle.isEmpty()&&!(category+" "+name).contains(needle,Qt::CaseInsensitive))continue;
            if(category!=last){group=new QTreeWidgetItem(tree_,{category});group->setFirstColumnSpanned(true);last=category;}
            const auto priority=row.priority==LinWinSettings::Priority::Visual?tr("1 · Visual"):row.priority==LinWinSettings::Priority::Ease?tr("2 · Ease of use"):tr("Deferred");
            const auto state=route.isEmpty()?(row.priority==LinWinSettings::Priority::Deferred?tr("Disabled · deferred"):tr("Disabled · pending")):route=="background"?tr("Available"):tr("Partial · existing controls");
            auto *item=new QTreeWidgetItem(group,{name,priority,state});item->setData(0,Qt::UserRole,route);
        }
        if(!needle.isEmpty())tree_->expandAll();else for(int i=0;i<tree_->topLevelItemCount();++i)if(tree_->topLevelItem(i)->text(0)=="Personalization")tree_->topLevelItem(i)->setExpanded(true);
        updateSelection();
    }
    void updateSelection(){
        auto *item=tree_->currentItem();const auto route=item?item->data(0,Qt::UserRole).toString():QString();
        openButton_->setEnabled(!route.isEmpty());detail_->setText(!item?tr("Select a setting."):!item->parent()?tr("Expand this category to view its settings."):route.isEmpty()?tr("This setting is visible for navigation but its action is disabled."):tr("Open the implemented LinWin controls for this setting."));
    }
    void activate(){auto *item=tree_->currentItem();if(!item)return;const auto route=item->data(0,Qt::UserRole).toString();if(!route.isEmpty())open_(route);}
    QLineEdit *search_=nullptr;QTreeWidget *tree_=nullptr;QLabel *detail_=nullptr;QPushButton *openButton_=nullptr;std::function<void(QString)> open_;
};
