// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QStringList>

class QFileSystemModel;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeView;

class FileManagerWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FileManagerWidget(QWidget *parent = nullptr);

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
    void showContextMenu(const QPoint &position);

    QFileSystemModel *model_ = nullptr;
    QTreeView *view_ = nullptr;
    QLineEdit *location_ = nullptr;
    QLabel *status_ = nullptr;
    QPushButton *backButton_ = nullptr;
    QString currentPath_;
    QStringList visitedPaths_;
    qsizetype visitedPathIndex_ = -1;
};
