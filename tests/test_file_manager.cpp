// SPDX-License-Identifier: GPL-3.0-or-later
#include "file_manager_widget.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class FileManagerTests final : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        QVERIFY(settingsDirectory_.isValid());
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory_.path());
    }

    void quickAccessShowsHomeAndMountedRoot()
    {
        QSettings settings;
        settings.clear();
        FileManagerWidget widget;
        const auto *places = widget.findChild<QListWidget *>(QStringLiteral("filePlaces"));
        QVERIFY(places != nullptr);

        const QString home = QFileInfo(QDir::homePath()).canonicalFilePath();
        bool hasHome = false;
        bool hasComputer = false;
        for (int i = 0; i < places->count(); ++i) {
            const QListWidgetItem *item = places->item(i);
            const QString path = item->data(Qt::UserRole).toString();
            if (path == home) hasHome = true;
            if (path == QStringLiteral("/")) hasComputer = true;
        }
        QVERIFY(hasHome);
        QVERIFY(hasComputer);
        QVERIFY(widget.findChild<QPushButton *>(QStringLiteral("pinCurrentFolderButton")) != nullptr);
    }

    void currentFolderCanBePinnedAndUnpinned()
    {
        QSettings settings;
        settings.clear();
        FileManagerWidget widget;
        auto *places = widget.findChild<QListWidget *>(QStringLiteral("filePlaces"));
        auto *pinButton = widget.findChild<QPushButton *>(QStringLiteral("pinCurrentFolderButton"));
        QVERIFY(places != nullptr);
        QVERIFY(pinButton != nullptr);

        QTest::mouseClick(pinButton, Qt::LeftButton);
        const QStringList pinned = settings.value(QStringLiteral("files/pinnedFolders")).toStringList();
        QCOMPARE(pinned.size(), 1);
        bool foundPinnedHome = false;
        for (int i = 0; i < places->count(); ++i) {
            const QListWidgetItem *item = places->item(i);
            if (item->data(Qt::UserRole + 1).toBool() && item->data(Qt::UserRole).toString() == pinned.front())
                foundPinnedHome = true;
        }
        QVERIFY(foundPinnedHome);

        QTest::mouseClick(pinButton, Qt::LeftButton);
        QVERIFY(settings.value(QStringLiteral("files/pinnedFolders")).toStringList().isEmpty());
    }

    void searchFiltersNamesWithoutChangingFolder()
    {
        FileManagerWidget widget;
        auto *search = widget.findChild<QLineEdit *>(QStringLiteral("fileSearch"));
        auto *model = widget.findChild<QFileSystemModel *>(QStringLiteral("fileSystemModel"));
        QVERIFY(search != nullptr);
        QVERIFY(model != nullptr);
        const QString folder = model->rootPath();

        search->setText(QStringLiteral("quarterly report"));
        QCOMPARE(model->nameFilters(), QStringList{QStringLiteral("*quarterly report*")});
        QCOMPARE(model->rootPath(), folder);

        search->clear();
        QVERIFY(model->nameFilters().isEmpty());
    }

private:
    QTemporaryDir settingsDirectory_;
};

QTEST_MAIN(FileManagerTests)
#include "test_file_manager.moc"
