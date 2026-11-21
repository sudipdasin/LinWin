// SPDX-License-Identifier: GPL-3.0-or-later
#include "file_manager_widget.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include <iostream>

namespace {
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
    QCoreApplication::setApplicationName(QStringLiteral("file manager"));
    QTemporaryDir settingsDirectory;
    if (!settingsDirectory.isValid()) {
        std::cerr << "Could not create isolated settings directory\n";
        return 1;
    }
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
    QSettings settings;

    FileManagerWidget widget;
    auto *places = widget.findChild<QListWidget *>(QStringLiteral("filePlaces"));
    auto *pinButton = widget.findChild<QPushButton *>(QStringLiteral("pinCurrentFolderButton"));
    auto *search = widget.findChild<QLineEdit *>(QStringLiteral("fileSearch"));
    auto *model = widget.findChild<QFileSystemModel *>(QStringLiteral("fileSystemModel"));
    bool passed = true;
    passed &= expect(places != nullptr && pinButton != nullptr,
                     "File Manager must expose the Quick access sidebar and pin button");
    passed &= expect(search != nullptr && model != nullptr,
                     "File Manager must expose the current-folder search and filesystem model");
    if (!passed) return 1;

    const QString home = QFileInfo(QDir::homePath()).canonicalFilePath();
    bool hasHome = false;
    bool hasComputer = false;
    for (int i = 0; i < places->count(); ++i) {
        const QListWidgetItem *item = places->item(i);
        const QString path = item->data(Qt::UserRole).toString();
        hasHome = hasHome || path == home;
        hasComputer = hasComputer || path == QStringLiteral("/");
    }
    passed &= expect(hasHome, "Quick access must include the current user's Home folder");
    passed &= expect(hasComputer, "Devices must include a Computer root location");

    pinButton->click();
    settings.sync();
    const QStringList pinned = settings.value(QStringLiteral("files/pinnedFolders")).toStringList();
    passed &= expect(pinned.size() == 1, "Pin button must persist one current folder");
    bool foundPinnedHome = false;
    for (int i = 0; i < places->count(); ++i) {
        const QListWidgetItem *item = places->item(i);
        foundPinnedHome = foundPinnedHome
            || (item->data(Qt::UserRole + 1).toBool() && item->data(Qt::UserRole).toString() == home);
    }
    passed &= expect(foundPinnedHome, "A pinned standard folder must remain marked removable in Quick access");
    pinButton->click();
    settings.sync();
    passed &= expect(settings.value(QStringLiteral("files/pinnedFolders")).toStringList().isEmpty(),
                     "Pin button must remove a previously pinned folder");

    const QString folder = model->rootPath();
    search->setText(QStringLiteral("quarterly report"));
    passed &= expect(model->nameFilters() == QStringList{QStringLiteral("*quarterly report*")},
                     "Search should filter visible names in the current folder");
    passed &= expect(model->rootPath() == folder, "Search should not change the active folder");
    search->clear();
    passed &= expect(model->nameFilters().isEmpty(), "Clearing search should restore all folder names");
    settings.clear();
    return passed ? 0 : 1;
}
