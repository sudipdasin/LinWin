// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "system_metrics.h"
#include "weather_widget.h"
#include <QCalendarWidget>
#include <QTableView>
#include <QHeaderView>
#include <QSizePolicy>
#include <QDateTime>
#include <QFile>
#include <QFrame>
#include <QHideEvent>
#include <QResizeEvent>
#include <QLabel>
#include <QShowEvent>
#include <QStorageInfo>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <array>
class DesktopWidgets final : public QWidget {
public:
    explicit DesktopWidgets(QWidget *parent=nullptr) : QWidget(parent) {
        setObjectName(QStringLiteral("neonWidgets"));
        setFixedWidth(292);
        auto *layout=new QVBoxLayout(this);
        layout->setContentsMargins(0,14,0,0); layout->setSpacing(12);
        clock_=new QLabel(this); clock_->setObjectName(QStringLiteral("widgetClock"));
        clock_->setAlignment(Qt::AlignVCenter|Qt::AlignLeft); clock_->setTextFormat(Qt::RichText); clock_->setFixedHeight(112); layout->addWidget(clock_);
        auto *calendar=new QCalendarWidget(this);
        calendar->setObjectName(QStringLiteral("linwinDesktopCalendar"));
        calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        calendar->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
        calendar->setGridVisible(false);
        calendar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        calendar->setFixedHeight(246);
        if(auto *table=calendar->findChild<QTableView *>()) {
            table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            table->horizontalHeader()->setMinimumSectionSize(12);
            table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            table->verticalHeader()->setMinimumSectionSize(12);
        }
        layout->addWidget(calendar);
        auto *system=new QFrame(this); system->setObjectName(QStringLiteral("widgetMetrics"));
        auto *metrics=new QVBoxLayout(system);
        metrics->setContentsMargins(10,10,10,10); metrics->setSpacing(5);
        auto *title=new QLabel(tr("System monitor"),system);
        title->setObjectName(QStringLiteral("metricTitle")); metrics->addWidget(title);
        const QStringList names{tr("CPU"),tr("Memory"),tr("Disk")};
        for (int i=0;i<3;++i) {
            auto *row=new QHBoxLayout;
            row->addWidget(new QLabel(names.at(i),system)); row->addStretch();
            values_[i]=new QLabel(tr("Unavailable"),system);
            row->addWidget(values_[i]); metrics->addLayout(row);
            bars_[i]=new QProgressBar(system);
            bars_[i]->setObjectName(QStringLiteral("linwinMetricBar_%1").arg(i));
            bars_[i]->setRange(0,100); bars_[i]->setTextVisible(false);
            bars_[i]->setFixedHeight(5);
            bars_[i]->setAccessibleName(names.at(i));
            metrics->addWidget(bars_[i]);
        }
        layout->addWidget(system);
        weather_=new WeatherWidget(this);layout->addWidget(weather_);layout->addStretch();
        setStyleSheet(QStringLiteral(
            "#neonWidgets { background:transparent; }"
            "#widgetClock,#widgetMetrics,#widgetWeather,QCalendarWidget { background:#092740; color:#e5f7ff; border:1px solid #148eb7; border-radius:12px; }"
            "#widgetClock { font-size:24px; font-weight:600; padding:12px; }"
            "#widgetMetrics { font-size:12px; }"
            "#widgetMetrics QLabel, #widgetWeather QLabel { background:transparent; color:#dbefff; }"
            "#widgetWeather QPushButton { background:#163b59; color:#e5f7ff; border:1px solid #286181; border-radius:6px; padding:4px; }"
            "#metricTitle { font-size:14px; font-weight:600; }"
            "#widgetMetrics QProgressBar { background:#203f57; border:0; border-radius:2px; }"
            "#widgetMetrics QProgressBar::chunk { background:#25bde4; border-radius:2px; }"
            "QCalendarWidget QWidget { background:#092740; color:#e5f7ff; }"
            "QCalendarWidget QAbstractItemView { background:#092740; alternate-background-color:#092740; color:#e5f7ff; selection-background-color:#087fbc; selection-color:white; font-size:12px; border:0; }"
            "QCalendarWidget QHeaderView::section { background:#092740; color:#c6e7f9; border:0; padding:0; }"
            "QCalendarWidget QToolButton { background:#092740; color:#e5f7ff; border:0; padding:3px; }"
        ));
        timer_.setInterval(2000);
        connect(&timer_,&QTimer::timeout,this,[this]{refresh();});
    }
    bool pollingActive() const { return timer_.isActive(); }
protected:
    void resizeEvent(QResizeEvent *e) override { QWidget::resizeEvent(e); if(weather_) weather_->setVisible(height()>=700); }
    void showEvent(QShowEvent *e) override { QWidget::showEvent(e); previous_.reset(); refresh(); timer_.start(); }
    void hideEvent(QHideEvent *e) override { timer_.stop(); previous_.reset(); QWidget::hideEvent(e); }
private:
    static QString percent(std::optional<int> value) {
        return value ? QString::number(*value)+QStringLiteral("%") : QObject::tr("Unavailable");
    }
    void refresh() {
        const auto now=QDateTime::currentDateTime();
        clock_->setText(QStringLiteral("<span style='font-size:38px;font-weight:600'>%1</span> <span style='font-size:14px'>%2</span><br/><span style='font-size:16px;font-weight:400'>%3</span>")
            .arg(now.toString(QStringLiteral("h:mm")),now.toString(QStringLiteral("AP")),now.toString(QStringLiteral("ddd, MMM d, yyyy")).toHtmlEscaped()));
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
        const std::array<std::optional<int>,3> metrics{cpuPercent,ram,diskPercent};
        for (std::size_t i=0;i<metrics.size();++i) {
            values_[i]->setText(percent(metrics[i]));
            bars_[i]->setValue(metrics[i].value_or(0));
            bars_[i]->setEnabled(metrics[i].has_value());
            bars_[i]->setToolTip(percent(metrics[i]));
        }
    }
    WeatherWidget *weather_=nullptr;
    QLabel *clock_=nullptr;
    std::array<QLabel*,3> values_{};
    std::array<QProgressBar*,3> bars_{};
    QTimer timer_; std::optional<LinWinMetrics::Cpu> previous_;
};
