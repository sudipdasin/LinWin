// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVector>

class QFrame;
class QCheckBox;
class QGridLayout;
class QLineEdit;
class QLabel;
class QResizeEvent;
class QPushButton;
class QScrollArea;
class QSlider;
class QProcess;
class QTimer;

class SettingsHubWidget final : public QWidget {
    Q_OBJECT
public:
    explicit SettingsHubWidget(QWidget *parent = nullptr);
    void focusSearch();
    int textScalePercent() const;
    void setTextScalePercent(int percent);

signals:
    void componentEnabledChanged(const QString &component, bool enabled);
    void textScaleChanged(int percent);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    struct LaunchTarget {
        QString program;
        QStringList arguments;
    };
    struct CategoryCard {
        QFrame *frame = nullptr;
        QString searchText;
    };

    void filterCategories(const QString &query);
    void arrangeCards();
    void openCategory(int index);
    void updateTextScaleStylesheet(int percent);
    void startUpdateCheck(bool manual);
    void startUpdateInstall(const QString &tag);

    QLineEdit *search_ = nullptr;
    QLabel *resultStatus_ = nullptr;
    QLabel *emptyState_ = nullptr;
    QLabel *textScaleValue_ = nullptr;
    QSlider *textScaleSlider_ = nullptr;
    QLabel *updateStatus_ = nullptr;
    QPushButton *checkUpdatesButton_ = nullptr;
    QCheckBox *automaticUpdatesCheck_ = nullptr;
    QProcess *updateProcess_ = nullptr;
    QTimer *automaticUpdateTimer_ = nullptr;
    bool updateCheckManual_ = false;
    bool installingUpdate_ = false;
    QString pendingUpdateTag_;
    QString baseStyleSheet_;
    QGridLayout *grid_ = nullptr;
    QVector<CategoryCard> cards_;
    QVector<LaunchTarget> targets_;
};
