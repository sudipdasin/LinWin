// SPDX-License-Identifier: GPL-3.0-or-later
#include "settings_hub_widget.h"

#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QSettings>
#include <QSlider>
#include <QPushButton>
#include <QRegularExpression>
#include <QVector>
#include <QDialog>
#include <QTimer>
#include <QComboBox>
#include <QTableWidget>
#include <QGuiApplication>
#include <QTreeWidget>
#include "windows10_settings_catalog.h"
#include <QTemporaryDir>
#include <QImage>

#include <iostream>

namespace {
int visibleCards(const SettingsHubWidget &widget)
{
    int count = 0;
    const auto cards = widget.findChildren<QFrame *>(QRegularExpression(QStringLiteral("^settingsCard_")));
    for (const QFrame *card : cards)
        if (!card->isHidden()) ++count;
    return count;
}

bool expect(bool condition, const char *message)
{
    if (condition) return true;
    std::cerr << message << '\n';
    return false;
}
}

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("LinWin tests"));
    QCoreApplication::setApplicationName(QStringLiteral("settings hub"));
    QSettings settings;
    settings.clear();
    SettingsHubWidget widget;
    widget.resize(900, 700);
    widget.show();
    application.processEvents();

    bool passed = true;
    auto *themes = widget.findChild<QComboBox *>(QStringLiteral("linwinThemeSelector"));
    passed &= expect(themes && themes->count() == 7 && themes->currentData().toString() == QStringLiteral("neon"), "Seven themes with Neon Fluent as default.");
    if (themes) {
        themes->setCurrentIndex(themes->findData(QStringLiteral("minimal")));
        passed &= expect(settings.value(QStringLiteral("appearance/theme")).toString() == QStringLiteral("minimal"), "Theme choice persists.");
        themes->setCurrentIndex(0);
    }
    const auto cards = widget.findChildren<QFrame *>(QRegularExpression(QStringLiteral("^settingsCard_")));
    passed &= expect(cards.size() == 12, "expected all 12 settings categories");
    passed &= expect(visibleCards(widget) == 12, "all categories should be visible before searching");

    auto *filesToggle = widget.findChild<QCheckBox *>(QStringLiteral("componentToggle_files"));
    auto *tasksToggle = widget.findChild<QCheckBox *>(QStringLiteral("componentToggle_task-manager"));
    auto *textScale = widget.findChild<QSlider *>(QStringLiteral("textScaleSlider"));
    auto *textScaleValue = widget.findChild<QLabel *>(QStringLiteral("textScaleValue"));
    auto *textScaleReset = widget.findChild<QPushButton *>(QStringLiteral("textScaleReset"));
    passed &= expect(filesToggle != nullptr && filesToggle->isChecked(),
                     "file manager should be enabled by default");
    passed &= expect(tasksToggle != nullptr && tasksToggle->isChecked(),
                     "Task Manager should be enabled by default");
    if (filesToggle != nullptr) filesToggle->setChecked(false);
    passed &= expect(!settings.value(QStringLiteral("components/files"), true).toBool(),
                     "component changes should persist to account settings");
    SettingsHubWidget reopened;
    auto *reopenedFilesToggle = reopened.findChild<QCheckBox *>(QStringLiteral("componentToggle_files"));
    passed &= expect(reopenedFilesToggle != nullptr && !reopenedFilesToggle->isChecked(),
                     "saved component preferences should load when settings reopen");

    passed &= expect(textScale != nullptr && textScaleValue != nullptr && textScaleReset != nullptr,
                     "settings should expose text-size controls");
    if (textScale != nullptr && textScaleValue != nullptr && textScaleReset != nullptr) {
        passed &= expect(textScale->minimum() == 85 && textScale->maximum() == 150,
                         "text scaling should stay within supported bounds");
        textScale->setValue(125);
        passed &= expect(textScaleValue->text() == QStringLiteral("125%")
                         && settings.value(QStringLiteral("appearance/textScalePercent")).toInt() == 125,
                         "text size should update its label and persist per account");
        passed &= expect(widget.styleSheet().contains(QStringLiteral("font-size: 20px")),
                         "text scaling should update LinWin settings typography");
        textScaleReset->click();
        passed &= expect(textScale->value() == 100
                         && settings.value(QStringLiteral("appearance/textScalePercent")).toInt() == 100,
                         "Reset should restore the standard text size");
        passed &= expect(widget.styleSheet().contains(QStringLiteral("font-size: 16px")),
                         "reset should restore default LinWin typography");
    }
    settings.clear();

    auto *info = widget.findChild<QPushButton *>(QStringLiteral("linwinDisplayInfoButton"));
    passed &= expect(info != nullptr, "Display information must be available without a host control panel.");
    if (info) {
        QTimer::singleShot(0, &widget, [&]() {
            auto *dialog = widget.findChild<QDialog *>(QStringLiteral("linwinDisplayInformation"));
            auto *table = dialog ? dialog->findChild<QTableWidget *>(QStringLiteral("linwinDisplayTable")) : nullptr;
            passed &= expect(table && table->rowCount() == QGuiApplication::screens().size(), "Display info must enumerate real Qt screens.");
            if (dialog) dialog->reject();
            else QApplication::closeAllWindows();
        });
        info->click();
    }

    auto *search = widget.findChild<QLineEdit *>(QStringLiteral("settingsSearch"));
    passed &= expect(search != nullptr, "search field should have a stable object name");
    if (search != nullptr) {
        search->setText(QStringLiteral("bluetooth"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "Bluetooth search should narrow to one category");

        search->setText(QStringLiteral("wifi"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "Wi-Fi synonym should find Network settings");

        search->setText(QStringLiteral("linwin updates"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 1, "LinWin update search should find its own category.");
        bool foundUpdateCard = false;
        for (const QFrame *card : cards)
            if (!card->isHidden() && card->property("categoryTitle").toString() == QStringLiteral("LinWin Updates"))
                foundUpdateCard = true;
        passed &= expect(foundUpdateCard, "LinWin Updates must be distinct from host software updates.");
        for (QFrame *card : cards) {
            if (card->isHidden() || card->property("categoryTitle").toString() != QStringLiteral("LinWin Updates")) continue;
            auto *open = card->findChild<QPushButton *>(QStringLiteral("settingsOpenButton"));
            passed &= expect(open && open->isEnabled(), "Embedded update page must not depend on companion installation.");
            if (open && open->isEnabled()) {
                QTimer::singleShot(0, &widget, [&]() {
                    auto *dialog = widget.findChild<QDialog *>(QStringLiteral("linwinEmbeddedUpdateDialog"));
                    passed &= expect(dialog && dialog->findChild<QPushButton *>(QStringLiteral("linwinCheckForUpdates")), "Update opens a native LinWin dialog.");
                    if (dialog) dialog->reject();
                    else QApplication::closeAllWindows();
                });
                open->click();
            }
        }

        search->setText(QStringLiteral("no matching setting"));
        application.processEvents();
        passed &= expect(visibleCards(widget) == 0, "unmatched search should hide all cards");
        auto *empty = widget.findChild<QLabel *>(QStringLiteral("settingsEmptyState"));
        passed &= expect(empty != nullptr && !empty->isHidden(), "unmatched search should show helpful empty state");

        search->clear();
        application.processEvents();
        passed &= expect(visibleCards(widget) == 12, "clearing search should restore all categories");
    }
    search->setText(QStringLiteral("wallpaper"));application.processEvents();
    bool opened=false;
    for(auto *card:cards)if(card->property("categoryTitle").toString()=="Personalization"){
        auto *open=card->findChild<QPushButton *>(QStringLiteral("settingsOpenButton"));
        if(open&&open->isEnabled()){open->click();opened=true;}
    }
    auto *page=widget.findChild<QWidget *>(QStringLiteral("linwinBackgroundSettings"));
    auto *mode=widget.findChild<QComboBox *>(QStringLiteral("linwinBackgroundMode"));
    auto *fit=widget.findChild<QComboBox *>(QStringLiteral("linwinBackgroundFit"));
    auto *apply=widget.findChild<QPushButton *>(QStringLiteral("linwinBackgroundApply"));
    passed &= expect(opened&&page&&page->isVisible()&&mode&&mode->count()==4&&fit&&fit->count()==6&&apply,"Personalization must open an integrated Background page.");
    if(mode&&apply&&page){
        int changed=0;QObject::connect(&widget,&SettingsHubWidget::backgroundChanged,&widget,[&changed]{++changed;});
        mode->setCurrentIndex(mode->findData("solid"));apply->click();
        passed &= expect(changed==1&&settings.value("background/mode").toString()=="solid","Applying background must persist and emit a live-change signal.");
        widget.focusSearch();widget.showBackground();
        passed &= expect(mode->currentData().toString()=="solid","Reopened Background page must remember the saved mode.");
        mode->setCurrentIndex(mode->findData("picture"));apply->click();
        passed &= expect(changed==1&&settings.value("background/mode").toString()=="solid","Missing picture must not overwrite saved background.");
        mode->setCurrentIndex(mode->findData("theme"));apply->click();
        widget.focusSearch();
        passed &= expect(!page->isVisible(),"Back to Settings must return to category overview.");
    }
    widget.showWindowsCatalog();application.processEvents();
    auto *tree=widget.findChild<QTreeWidget *>("linwinCatalogTree");
    auto *find=widget.findChild<QLineEdit *>("linwinCatalogSearch");
    auto *openSetting=widget.findChild<QPushButton *>("linwinCatalogOpen");
    passed &= expect(tree&&find&&openSetting&&tree->topLevelItemCount()==13,"Windows 10 settings catalog must expose 13 categories.");
    if(tree&&find&&openSetting){
        int entries=0;for(int i=0;i<tree->topLevelItemCount();++i)entries+=tree->topLevelItem(i)->childCount();
        passed &= expect(entries==static_cast<int>(LinWinSettings::catalog.size()),"Every catalog page must be visible.");
        find->setText("VPN");application.processEvents();
        auto *item=tree->topLevelItemCount()?tree->topLevelItem(0)->child(0):nullptr;
        if(item)tree->setCurrentItem(item);
        passed &= expect(item&&!openSetting->isEnabled(),"Deferred VPN action must be disabled.");
        find->setText("Background");application.processEvents();
        item=nullptr;for(int i=0;i<tree->topLevelItemCount();++i)for(int j=0;j<tree->topLevelItem(i)->childCount();++j)if(tree->topLevelItem(i)->child(j)->text(0)=="Background")item=tree->topLevelItem(i)->child(j);
        if(item)tree->setCurrentItem(item);
        passed &= expect(item&&openSetting->isEnabled(),"Implemented Background route must be enabled.");
        if(item&&openSetting->isEnabled()){openSetting->click();passed &= expect(page&&page->isVisible(),"Catalog Background opens native page.");}
    }
    widget.showVisualPage("colors");application.processEvents();
    auto *colorMode=widget.findChild<QComboBox *>("linwinShellColorMode");
    auto *transparency=widget.findChild<QCheckBox *>("transparency");
    int visualChanges=0;QObject::connect(&widget,&SettingsHubWidget::visualChanged,&widget,[&]{++visualChanges;});
    passed &= expect(colorMode&&colorMode->isVisible()&&transparency&&transparency->isVisible(),"Colors page exposes real mode and glass controls.");
    if(colorMode&&transparency){colorMode->setCurrentIndex(colorMode->findData("dark"));transparency->setChecked(false);}
    widget.showVisualPage("start");application.processEvents();
    auto *full=widget.findChild<QCheckBox *>("startFullscreen");
    passed &= expect(full&&full->isVisible(),"Start settings page exposes fullscreen.");if(full)full->setChecked(true);
    widget.showVisualPage("taskbar");application.processEvents();
    auto *small=widget.findChild<QCheckBox *>("smallTaskbar");
    passed &= expect(small&&small->isVisible(),"Taskbar settings page exposes compact mode.");if(small)small->setChecked(true);
    passed &= expect(visualChanges==4&&settings.value("visual/shellMode").toString()=="dark"&&!settings.value("visual/transparency").toBool()&&settings.value("visual/startFullscreen").toBool()&&settings.value("visual/smallTaskbar").toBool(),"Visual controls persist and emit live-change events.");
    widget.showVisualPage("colors");
    passed &= expect(colorMode&&colorMode->currentData().toString()=="dark","Reopened visual page restores preferences.");
    settings.remove("visual");
    return passed ? 0 : 1;
}
