// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QStringList>
#include <QWidget>

typedef struct _GAppInfo GAppInfo;

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class AppLauncherWidget final : public QWidget {
    Q_OBJECT
public:
    explicit AppLauncherWidget(QWidget *parent = nullptr);
    ~AppLauncherWidget() override;
    void focusSearch();

private slots:
    void filterApplications(const QString &query);
    void launchSelectedApplication();
    void togglePinnedApplication();
    void updatePinButton();

private:
    void loadApplications();
    void refreshList();
    void savePinnedApplications() const;

    QLineEdit *searchField_ = nullptr;
    QListWidget *applicationList_ = nullptr;
    QListWidget *pinnedList_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QPushButton *pinButton_ = nullptr;
    QHash<QString, GAppInfo *> applications_;
    QStringList pinnedApplicationIds_;
};
