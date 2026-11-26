// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QStyle>
#include <QPixmap>
namespace LinWinVisual {
struct WindowCard {
    QString title;
    QString detail;
    QString key;
    QIcon preview;
};
class WindowOverview final : public QDialog {
public:
    explicit WindowOverview(const QList<WindowCard>& cards,QWidget *parent=nullptr):QDialog(parent) {
        setObjectName(QStringLiteral("linwinWindowOverview"));
        setWindowTitle(tr("LinWin Task View"));
        resize(740,440);
        auto *layout=new QVBoxLayout(this);
        auto *heading=new QLabel(tr("Task View"),this);
        heading->setStyleSheet(QStringLiteral("font-size:24px;font-weight:600;"));
        layout->addWidget(heading);
        auto *description=new QLabel(tr("Choose an open window to return to it. External apps show an icon instead of a screenshot."),this);
        description->setWordWrap(true);layout->addWidget(description);
        list_=new QListWidget(this);
        list_->setObjectName(QStringLiteral("linwinWindowCards"));
        list_->setViewMode(QListView::IconMode);
        list_->setResizeMode(QListView::Adjust);
        list_->setMovement(QListView::Static);
        list_->setSelectionMode(QAbstractItemView::SingleSelection);
        list_->setIconSize(QSize(192,108));
        list_->setGridSize(QSize(220,154));
        list_->setWordWrap(true);
        list_->setSpacing(4);
        for (const auto &card:cards) {
            auto *item=new QListWidgetItem(card.preview,card.title,list_);
            item->setData(Qt::UserRole,card.key);
            item->setToolTip(card.title+QStringLiteral("\n")+card.detail);
            item->setData(Qt::AccessibleDescriptionRole,card.detail);
        }
        layout->addWidget(list_,1);
        if(cards.isEmpty()) {
            auto *empty=new QLabel(tr("No open windows in the current LinWin window list."),this);
            empty->setAlignment(Qt::AlignCenter);layout->addWidget(empty);
        } else list_->setCurrentRow(0);
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Open|QDialogButtonBox::Cancel,this);
        auto *open=buttons->button(QDialogButtonBox::Open);
        open->setEnabled(list_->currentItem()!=nullptr);
        connect(list_,&QListWidget::currentItemChanged,this,[open](QListWidgetItem *current,QListWidgetItem*){open->setEnabled(current!=nullptr);});
        connect(buttons,&QDialogButtonBox::accepted,this,[this]{choose();});
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        connect(list_,&QListWidget::itemActivated,this,[this](QListWidgetItem*){choose();});
        layout->addWidget(buttons);
        list_->setFocus();
    }
    QString chosenKey() const {return chosen_;}
private:
    void choose() {
        if(!list_->currentItem()) return;
        chosen_=list_->currentItem()->data(Qt::UserRole).toString();
        if(!chosen_.isEmpty()) accept();
    }
    QListWidget *list_=nullptr;
    QString chosen_;
};
}
