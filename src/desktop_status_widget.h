// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "desktop_status.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QProcess>
#include <QMessageBox>
#include <QTimer>
#include <QShowEvent>
#include <QHideEvent>
#include <QPainter>
#include <QPainterPath>
#include <utility>
#include <QIcon>
#include <functional>
class DesktopStatus final:public QFrame {
public:
    explicit DesktopStatus(QWidget *parent,std::function<void()> quick,std::function<void()> overview):QFrame(parent) {
        setObjectName(QStringLiteral("linwinStatusArea"));
        auto *layout=new QHBoxLayout(this);layout->setContentsMargins(3,0,3,0);layout->setSpacing(2);
        auto make=[this,layout](const QString &id,const QString &label){
            auto *b=new QPushButton(this);b->setObjectName(id);b->setAccessibleName(label);
            b->setIconSize(QSize(22,22));b->setFixedSize(33,36);layout->addWidget(b);return b;
        };
        auto *more=make("linwinStatusActions",tr("Desktop controls"));
        more->setIcon(draw(0));more->setToolTip(tr("Quick controls and Task View"));
        auto *menu=new QMenu(more);
        menu->addAction(tr("Quick Controls"),this,std::move(quick));
        menu->addAction(tr("Task View"),this,std::move(overview));
        menu->addSeparator();
        menu->addAction(tr("Power settings"),this,[this]{launch({QStringLiteral("xfce4-power-manager-settings")});});
        more->setMenu(menu);
        network_=make("linwinStatusNetwork",tr("Network status"));
        connect(network_,&QPushButton::clicked,this,[this]{launch({QStringLiteral("nm-connection-editor")});});
        auto *audio=make("linwinStatusAudio",tr("Open sound controls"));
        audio->setIcon(draw(2));audio->setToolTip(tr("Open host sound controls; volume is not monitored"));
        connect(audio,&QPushButton::clicked,this,[this]{launch({QStringLiteral("pavucontrol"),QStringLiteral("xfce4-mixer")});});
        battery_=make("linwinStatusBattery",tr("Battery status"));
        connect(battery_,&QPushButton::clicked,this,[this]{launch({QStringLiteral("xfce4-power-manager-settings")});});
        timer_.setInterval(10000);connect(&timer_,&QTimer::timeout,this,[this]{refresh();});
        refresh();
    }
    bool pollingActive() const{return timer_.isActive();}
protected:
    void showEvent(QShowEvent *e) override{QFrame::showEvent(e);refresh();timer_.start();}
    void hideEvent(QHideEvent *e) override{timer_.stop();QFrame::hideEvent(e);}
private:
    void launch(const QStringList &candidates){
        for(const auto &name:candidates){const auto path=QStandardPaths::findExecutable(name);if(!path.isEmpty()&&QProcess::startDetached(path,{}))return;}
        QMessageBox::information(this,tr("Desktop controls"),tr("The corresponding host utility is not installed."));
    }
    static QIcon draw(int kind,int level=0){
        QPixmap pix(44,44);pix.fill(Qt::transparent);QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing);p.scale(2,2);
        p.setPen(QPen(QColor("#e7f6ff"),1.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.setBrush(Qt::NoBrush);
        if(kind==0){p.drawLine(QPointF(6,13),QPointF(11,8));p.drawLine(QPointF(11,8),QPointF(16,13));}
        else if(kind==1){p.drawRoundedRect(QRectF(3,4,16,11),2,2);p.drawLine(11,15,11,19);p.drawLine(7,19,15,19);if(!level){p.setPen(QPen(QColor("#f5a861"),1.8));p.drawLine(3,19,19,3);}}
        else if(kind==2){QPainterPath path;path.moveTo(3,8);path.lineTo(7,8);path.lineTo(11,4);path.lineTo(11,18);path.lineTo(7,14);path.lineTo(3,14);path.closeSubpath();p.drawPath(path);p.drawArc(QRectF(8,6,10,10),-60*16,120*16);p.drawArc(QRectF(7,3,16,16),-55*16,110*16);}
        else {p.drawRoundedRect(QRectF(2,6,17,10),2,2);p.drawLine(21,9,21,13);p.fillRect(QRectF(4,8,13*qBound(0,level,100)/100.0,6),QColor("#50d4ed"));}
        p.end();return QIcon(pix);
    }
    void refresh(){
        bool linked=false;
        for(const auto &iface:QNetworkInterface::allInterfaces()){
            const auto f=iface.flags();
            if(f.testFlag(QNetworkInterface::IsUp)&&f.testFlag(QNetworkInterface::IsRunning)&&!f.testFlag(QNetworkInterface::IsLoopBack)&&!iface.addressEntries().isEmpty()){linked=true;break;}
        }
        network_->setIcon(draw(1,linked?1:0));
        network_->setToolTip(linked?tr("Active network interface (internet reachability not tested)"):tr("No active network interface detected"));
        const auto b=LinWinStatus::battery();battery_->setVisible(b.has_value());
        if(b){battery_->setIcon(draw(3,b->percent));battery_->setToolTip(tr("Battery: %1% · %2").arg(b->percent).arg(b->state));}
    }
    QPushButton *network_=nullptr,*battery_=nullptr;QTimer timer_;
};
