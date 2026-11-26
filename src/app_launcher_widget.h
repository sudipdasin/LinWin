// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QStringList>
#include <QWidget>
#include <QImage>

typedef struct _GAppInfo GAppInfo;

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QStackedWidget;
class QPaintEvent;

class AppLauncherWidget final : public QWidget {
    Q_OBJECT
public:
    explicit AppLauncherWidget(QWidget *parent = nullptr);
    ~AppLauncherWidget() override;
    void focusSearch();
    void setGlassBackdrop(const QImage &sample) { glassBackdrop_=sample;update(); }
protected:
    void paintEvent(QPaintEvent *event) override;
public:

signals:
    void toolRequested(int index);
    void exitRequested();
    void returnDesktopRequested();

private slots:
    void filterApplications(const QString &query);
    void launchSelectedApplication();
    void togglePinnedApplication();
    void updatePinButton();

private:
    void loadApplications();
    void refreshList();
    void addSuggestedPins();
    void refreshRecentDocuments();
    void savePinnedApplications() const;

    QImage glassBackdrop_;
    QStackedWidget *appViews_ = nullptr;
    QLineEdit *searchField_ = nullptr;
    QListWidget *applicationList_ = nullptr;
    QListWidget *pinnedList_ = nullptr;
    QListWidget *recentList_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QPushButton *pinButton_ = nullptr;
    QHash<QString, GAppInfo *> applications_;
    QStringList pinnedApplicationIds_;
};
