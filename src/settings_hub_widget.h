// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVector>

class QFrame;
class QGridLayout;
class QLineEdit;
class QLabel;
class QResizeEvent;
class QPushButton;
class QScrollArea;

class SettingsHubWidget final : public QWidget {
    Q_OBJECT
public:
    explicit SettingsHubWidget(QWidget *parent = nullptr);
    void focusSearch();

signals:
    void componentEnabledChanged(const QString &component, bool enabled);

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

    QLineEdit *search_ = nullptr;
    QLabel *resultStatus_ = nullptr;
    QLabel *emptyState_ = nullptr;
    QGridLayout *grid_ = nullptr;
    QVector<CategoryCard> cards_;
    QVector<LaunchTarget> targets_;
};
