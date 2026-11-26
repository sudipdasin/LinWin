// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "system_metrics.h"
#include <QCalendarWidget>
#include <QDateTime>
#include <QFile>
#include <QFrame>
#include <QHideEvent>
#include <QLabel>
#include <QShowEvent>
#include <QStorageInfo>
#include <QTimer>
#include <QVBoxLayout>
class DesktopWidgets final : public QWidget {
public:
    explicit DesktopWidgets(QWidget *parent=nullptr) : QWidget(parent) {
        setObjectName(QStringLiteral("neonWidgets"));
        setFixedWidth(236);
        auto *layout=new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0); layout->setSpacing(10);
        clock_=new QLabel(this); clock_->setObjectName(QStringLiteral("widgetClock"));
        clock_->setAlignment(Qt::AlignCenter); layout->addWidget(clock_);
        auto *calendar=new QCalendarWidget(this);
        calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        calendar->setGridVisible(false); layout->addWidget(calendar);
        system_=new QLabel(this); system_->setObjectName(QStringLiteral("widgetMetrics"));
        system_->setTextFormat(Qt::PlainText); system_->setWordWrap(true);
        layout->addWidget(system_); layout->addStretch();
        setStyleSheet(QStringLiteral(
            "#neonWidgets { background:transparent; }"
            "#widgetClock,#widgetMetrics,QCalendarWidget { background:#092740; color:#e5f7ff; border:1px solid #148eb7; border-radius:9px; padding:10px; }"
            "#widgetClock { font-size:20px; font-weight:600; }"
            "#widgetMetrics { font-size:13px; }"
            "QCalendarWidget QWidget { background:#092740; color:#e5f7ff; }"
            "QCalendarWidget QAbstractItemView { selection-background-color:#087fbc; selection-color:white; }"
        ));
        timer_.setInterval(2000);
        connect(&timer_,&QTimer::timeout,this,[this]{refresh();});
    }
protected:
    void showEvent(QShowEvent *e) override { QWidget::showEvent(e); previous_.reset(); refresh(); timer_.start(); }
    void hideEvent(QHideEvent *e) override { timer_.stop(); previous_.reset(); QWidget::hideEvent(e); }
private:
    static QString percent(std::optional<int> value) {
        return value ? QString::number(*value)+QStringLiteral("%") : QObject::tr("Unavailable");
    }
    void refresh() {
        clock_->setText(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss\nddd, dd MMM yyyy")));
        QFile stat(QStringLiteral("/proc/stat")); std::optional<int> cpuPercent;
        if (stat.open(QIODevice::ReadOnly)) {
            auto sample=LinWinMetrics::cpu(stat.readLine().toStdString());
            if (sample && previous_) cpuPercent=LinWinMetrics::usage(*previous_,*sample);
            previous_=sample;
        } else previous_.reset();
        QFile mem(QStringLiteral("/proc/meminfo")); std::optional<int> ram;
        if (mem.open(QIODevice::ReadOnly)) ram=LinWinMetrics::memory(mem.readAll().toStdString());
        QStorageInfo disk=QStorageInfo::root(); std::optional<int> diskPercent;
        if (disk.isValid() && disk.isReady() && disk.bytesTotal()>0 && disk.bytesAvailable()>=0 && disk.bytesAvailable()<=disk.bytesTotal())
            diskPercent=static_cast<int>(100.0*(1.0-static_cast<double>(disk.bytesAvailable())/static_cast<double>(disk.bytesTotal()))+0.5);
        system_->setText(tr("System monitor\nCPU    %1\nRAM    %2\nDisk    %3").arg(percent(cpuPercent),percent(ram),percent(diskPercent)));
    }
    QLabel *clock_=nullptr,*system_=nullptr;
    QTimer timer_; std::optional<LinWinMetrics::Cpu> previous_;
};
