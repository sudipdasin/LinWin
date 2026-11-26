// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "weather_forecast.h"
#include <QFrame>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QTimer>
#include <QShowEvent>
#include <QHideEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QPointer>
#include <QIcon>
#include <QLocale>
class WeatherWidget final:public QFrame {
public:
    explicit WeatherWidget(QWidget *parent=nullptr):QFrame(parent),network_(this) {
        setObjectName(QStringLiteral("widgetWeather"));
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(10,8,10,8);layout->setSpacing(4);
        auto *header=new QHBoxLayout;header->addWidget(new QLabel(tr("Weather"),this));header->addStretch();
        auto *configure=new QPushButton(tr("Set location"),this);configure->setObjectName(QStringLiteral("linwinWeatherConfigure"));
        header->addWidget(configure);layout->addLayout(header);
        auto *now=new QHBoxLayout;icon_=new QLabel(this);now->addWidget(icon_);
        summary_=new QLabel(tr("Not configured"),this);summary_->setTextFormat(Qt::PlainText);summary_->setWordWrap(true);now->addWidget(summary_,1);layout->addLayout(now);
        auto *forecast=new QHBoxLayout;
        for(int i=0;i<3;++i){days_[i]=new QLabel(this);days_[i]->setAlignment(Qt::AlignCenter);forecast->addWidget(days_[i],1);}
        layout->addLayout(forecast);
        credit_=new QLabel(tr("Open-Meteo · personal use"),this);credit_->setStyleSheet(QStringLiteral("font-size:10px;"));layout->addWidget(credit_);
        connect(configure,&QPushButton::clicked,this,[this]{configureLocation();});
        timer_.setInterval(30*60*1000);connect(&timer_,&QTimer::timeout,this,[this]{refresh();});
    }
    bool pollingActive() const{return timer_.isActive();}
protected:
    void showEvent(QShowEvent *e) override {QFrame::showEvent(e);refresh();}
    void hideEvent(QHideEvent *e) override {timer_.stop();cancel();QFrame::hideEvent(e);}
private:
    static QString condition(int code) {
        if(code==0)return tr("Clear");if(code<=3)return tr("Cloudy");if(code<=48)return tr("Fog");
        if(code<=67)return tr("Rain");if(code<=77)return tr("Snow");if(code<=86)return tr("Showers");return tr("Storm");
    }
    static QString iconName(int code) {
        if(code==0)return QStringLiteral("weather-clear");if(code<=3)return QStringLiteral("weather-few-clouds");
        if(code<=48)return QStringLiteral("weather-fog");if(code>=71&&code<=77)return QStringLiteral("weather-snow");
        return QStringLiteral("weather-showers");
    }
    void clearDays(){for(auto *day:days_)day->clear();icon_->clear();}
    void cancel(){if(reply_){auto *old=reply_.data();reply_.clear();old->abort();}}
    void configureLocation() {
        QSettings s;QDialog dialog(this);dialog.setWindowTitle(tr("Weather location"));
        auto *layout=new QFormLayout(&dialog);
        auto *consent=new QCheckBox(tr("Enable online weather for personal use"),&dialog);consent->setChecked(s.value("weather/enabled",false).toBool());layout->addRow(consent);
        auto *city=new QLineEdit(s.value("weather/city").toString(),&dialog);city->setMaxLength(80);layout->addRow(tr("City label"),city);
        auto *lat=new QDoubleSpinBox(&dialog);lat->setRange(-90,90);lat->setDecimals(2);lat->setValue(s.value("weather/latitude",0).toDouble());layout->addRow(tr("Approximate latitude"),lat);
        auto *lon=new QDoubleSpinBox(&dialog);lon->setRange(-180,180);lon->setDecimals(2);lon->setValue(s.value("weather/longitude",0).toDouble());layout->addRow(tr("Approximate longitude"),lon);
        auto *note=new QLabel(tr("Only the coordinates you enter are sent to Open-Meteo when enabled. No automatic location lookup. Commercial use needs the provider's appropriate license."),&dialog);note->setWordWrap(true);layout->addRow(note);
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);layout->addRow(buttons);
        connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()!=QDialog::Accepted)return;
        s.setValue("weather/enabled",consent->isChecked());
        if(consent->isChecked()){
            s.setValue("weather/city",city->text().trimmed());s.setValue("weather/latitude",lat->value());s.setValue("weather/longitude",lon->value());
        }else {s.remove("weather/city");s.remove("weather/latitude");s.remove("weather/longitude");}
        cancel();refresh();
    }
    void refresh() {
        if(!isVisible())return;
        QSettings s;
        if(!s.value("weather/enabled",false).toBool()){timer_.stop();clearDays();summary_->setText(tr("Set a city to show weather"));return;}
        if(reply_)return;
        bool latOk=false,lonOk=false;
        const double lat=s.value("weather/latitude").toDouble(&latOk),lon=s.value("weather/longitude").toDouble(&lonOk);
        if(!latOk||!lonOk||!std::isfinite(lat)||!std::isfinite(lon)||lat< -90||lat>90||lon< -180||lon>180){timer_.stop();clearDays();summary_->setText(tr("Invalid location"));return;}
        if(!timer_.isActive())timer_.start();
        QUrl url(QStringLiteral("https://api.open-meteo.com/v1/forecast"));QUrlQuery query;
        query.addQueryItem("latitude",QString::number(lat,'f',2));query.addQueryItem("longitude",QString::number(lon,'f',2));
        query.addQueryItem("current","temperature_2m,weather_code");query.addQueryItem("daily","weather_code,temperature_2m_max,temperature_2m_min");query.addQueryItem("forecast_days","3");query.addQueryItem("timezone","auto");url.setQuery(query);
        QNetworkRequest request(url);request.setTransferTimeout(12000);request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::SameOriginRedirectPolicy);
        body_.clear();clearDays();summary_->setText(tr("Updating weather…"));
        auto *reply=network_.get(request);reply_=reply;
        connect(reply,&QNetworkReply::readyRead,this,[this,reply]{
            if(reply_!=reply)return;body_+=reply->readAll();if(body_.size()>65536)reply->abort();
        });
        connect(reply,&QNetworkReply::finished,this,[this,reply]{
            if(reply_==reply){
                reply_.clear();body_+=reply->readAll();
                const auto parsed=reply->error()==QNetworkReply::NoError?LinWinWeather::parse(body_):std::nullopt;
                if(parsed){
                    const QString city=QSettings().value("weather/city",tr("Selected location")).toString();
                    summary_->setText(QStringLiteral("%1°C · %2\n%3").arg(parsed->current).arg(condition(parsed->code),city));
                    icon_->setPixmap(QIcon::fromTheme(iconName(parsed->code)).pixmap(40,40));
                    for(int i=0;i<3;++i){const auto &d=parsed->days[i];days_[i]->setText(QStringLiteral("%1\n%2° / %3°").arg(QLocale().toString(d.date,QStringLiteral("ddd"))).arg(d.high).arg(d.low));}
                } else {clearDays();summary_->setText(tr("Weather unavailable"));}
                body_.clear();
            }
            reply->deleteLater();
        });
    }
    QNetworkAccessManager network_;QPointer<QNetworkReply> reply_;QTimer timer_;QByteArray body_;
    QLabel *icon_=nullptr,*summary_=nullptr,*credit_=nullptr;QLabel *days_[3]{};
};
