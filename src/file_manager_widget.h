// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QStringList>

class QFileSystemModel;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeView;

class FileManagerWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FileManagerWidget(QWidget *parent = nullptr);
    void openLocation(const QString &path) { navigateTo(path); }

private:
    QStringList selectedPaths() const;
    void navigateTo(const QString &path, bool remember = true);
    void navigateBack();
    void updateSelectionStatus();
    void createFolder();
    void renameSelected();
    void copySelected(bool cut);
    void pasteClipboard();
    void moveToTrash();
    void createShortcut();
    void populatePlaces();
    void togglePinnedFolder();
    void showPlacesContextMenu(const QPoint &position);
    void showContextMenu(const QPoint &position);

    QFileSystemModel *model_ = nullptr;
    QListWidget *places_ = nullptr;
    QTreeView *view_ = nullptr;
    QLineEdit *location_ = nullptr;
    QLineEdit *search_ = nullptr;
    QLabel *status_ = nullptr;
    QPushButton *backButton_ = nullptr;
    QPushButton *pinFolderButton_ = nullptr;
    QString currentPath_;
    QStringList visitedPaths_;
    qsizetype visitedPathIndex_ = -1;
};
