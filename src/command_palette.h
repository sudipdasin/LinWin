// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QDialog>
#include <QIcon>
#include <utility>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>
#include <algorithm>
namespace LinWinVisual {
struct Command { QString id,title,keywords; QIcon icon; };
inline int commandScore(const Command &command,const QString &query) {
    const auto words=query.simplified().split(QLatin1Char(' '),Qt::SkipEmptyParts);
    int score=0;
    for(const auto &word:words) {
        if(command.title.startsWith(word,Qt::CaseInsensitive)) score+=3;
        else if(command.title.contains(word,Qt::CaseInsensitive)) score+=2;
        else if(command.keywords.contains(word,Qt::CaseInsensitive)) score+=1;
        else return -1;
    }
    return score;
}
class CommandPalette final:public QDialog {
public:
    explicit CommandPalette(QList<Command> commands,QWidget *parent=nullptr):QDialog(parent),commands_(std::move(commands)) {
        setObjectName(QStringLiteral("linwinCommandPalette"));
        setWindowTitle(tr("LinWin Command Palette"));resize(560,440);
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(20,18,20,18);layout->setSpacing(10);
        auto *title=new QLabel(tr("Command Palette"),this);title->setObjectName(QStringLiteral("pageTitle"));layout->addWidget(title);
        search_=new QLineEdit(this);search_->setObjectName(QStringLiteral("linwinCommandSearch"));
        search_->setAccessibleName(tr("Search desktop commands"));search_->setPlaceholderText(tr("Search tools, themes and workspace actions…"));
        search_->setClearButtonEnabled(true);search_->installEventFilter(this);layout->addWidget(search_);
        list_=new QListWidget(this);list_->setObjectName(QStringLiteral("linwinCommandResults"));
        list_->setIconSize(QSize(24,24));list_->setSpacing(3);layout->addWidget(list_,1);
        hint_=new QLabel(this);hint_->setWordWrap(true);layout->addWidget(hint_);
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Open|QDialogButtonBox::Cancel,this);
        open_=buttons->button(QDialogButtonBox::Open);open_->setDefault(false);open_->setAutoDefault(false);
        layout->addWidget(buttons);
        connect(search_,&QLineEdit::textChanged,this,[this]{refresh();});
        connect(search_,&QLineEdit::returnPressed,this,[this]{choose();});
        connect(list_,&QListWidget::itemActivated,this,[this](QListWidgetItem*){choose();});
        connect(list_,&QListWidget::currentItemChanged,this,[this](QListWidgetItem *item,QListWidgetItem*){open_->setEnabled(item!=nullptr);});
        connect(buttons,&QDialogButtonBox::accepted,this,[this]{choose();});
        connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
        refresh();search_->setFocus();
    }
    QString chosen() const{return chosen_;}
protected:
    bool eventFilter(QObject *object,QEvent *event) override {
        if(object==search_&&event->type()==QEvent::KeyPress) {
            auto *key=static_cast<QKeyEvent*>(event);
            if(key->key()==Qt::Key_Down||key->key()==Qt::Key_Up) {
                if(list_->count()>0)list_->setCurrentRow(qBound(0,list_->currentRow()+(key->key()==Qt::Key_Down?1:-1),list_->count()-1));
                return true;
            }
        }
        return QDialog::eventFilter(object,event);
    }
private:
    void refresh() {
        QList<QPair<int,int>> ranked;
        for(int i=0;i<commands_.size();++i) {
            const int score=commandScore(commands_[i],search_->text());
            if(score>=0)ranked.append({score,i});
        }
        std::stable_sort(ranked.begin(),ranked.end(),[](const auto &a,const auto &b){return a.first>b.first;});
        list_->clear();
        for(const auto &entry:ranked) {
            const auto &cmd=commands_[entry.second];
            auto *item=new QListWidgetItem(cmd.icon,cmd.title,list_);item->setData(Qt::UserRole,cmd.id);
        }
        if(list_->count()>0)list_->setCurrentRow(0);
        open_->setEnabled(list_->currentItem()!=nullptr);
        hint_->setText(list_->count()?tr("↑ ↓ Select · Enter Open · Esc Cancel"):tr("No matching commands"));
    }
    void choose(){if(auto *item=list_->currentItem()){chosen_=item->data(Qt::UserRole).toString();if(!chosen_.isEmpty())accept();}}
    QList<Command> commands_;
    QLineEdit *search_=nullptr;QListWidget *list_=nullptr;QLabel *hint_=nullptr;QPushButton *open_=nullptr;
    QString chosen_;
};
}
