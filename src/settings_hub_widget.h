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
class QSlider;
class QStackedWidget;
class BackgroundSettingsPage;
class WindowsSettingsCatalogPage;
class VisualSettingsPage;
class AppearanceLibraryPage;

class SettingsHubWidget final : public QWidget {
    Q_OBJECT
public:
    explicit SettingsHubWidget(QWidget *parent = nullptr);
    void focusSearch();
    void showBackground();
    void showWindowsCatalog();
    void showVisualPage(const QString &route);
    int textScalePercent() const;
    void setTextScalePercent(int percent);

signals:
    void componentEnabledChanged(const QString &component, bool enabled);
    void textScaleChanged(int percent);
    void themeChanged(const QString &id);
    void backgroundChanged();
    void visualChanged();

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

    QStackedWidget *sections_=nullptr;
    BackgroundSettingsPage *backgroundPage_=nullptr;
    WindowsSettingsCatalogPage *windowsCatalog_=nullptr;
    AppearanceLibraryPage *fontLibrary_=nullptr,*themeLibrary_=nullptr;
    VisualSettingsPage *colors_=nullptr,*startAppearance_=nullptr,*taskbarAppearance_=nullptr;
    QLineEdit *search_ = nullptr;
    QLabel *resultStatus_ = nullptr;
    QLabel *emptyState_ = nullptr;
    QLabel *textScaleValue_ = nullptr;
    QSlider *textScaleSlider_ = nullptr;
    QString baseStyleSheet_;
    QGridLayout *grid_ = nullptr;
    QVector<CategoryCard> cards_;
    QVector<LaunchTarget> targets_;
};
