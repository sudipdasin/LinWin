// SPDX-License-Identifier: GPL-3.0-or-later
#include <gio/gio.h>

#include "file_manager_widget.h"

#include <QApplication>
#include <QAction>
#include <QClipboard>
#include <QDir>
#include <QDesktopServices>
#include <QDialog>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

namespace {
constexpr auto kLinWinCutMime = "application/x-linwin-cut";
constexpr auto kGnomeFilesMime = "x-special/gnome-copied-files";

bool copyPath(const QString &source, const QString &destination, QString *errorText)
{
    const QFileInfo sourceInfo(source);
    if (!sourceInfo.exists() && !sourceInfo.isSymLink()) {
        *errorText = QObject::tr("The source no longer exists.");
        return false;
    }
    if (sourceInfo.isSymLink()) {
        if (QFile::link(sourceInfo.symLinkTarget(), destination)) {
            return true;
        }
        *errorText = QObject::tr("Could not create a link for %1.").arg(sourceInfo.fileName());
        return false;
    }
    if (sourceInfo.isDir()) {
        if (!QDir().mkpath(destination)) {
            *errorText = QObject::tr("Could not create the destination folder.");
            return false;
        }
        const QDir sourceDir(source);
        const QFileInfoList entries = sourceDir.entryInfoList(
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
            QDir::Name | QDir::DirsFirst);
        for (const QFileInfo &entry : entries) {
            const QString childDestination = QDir(destination).filePath(entry.fileName());
            if (!copyPath(entry.filePath(), childDestination, errorText)) {
                QDir(destination).removeRecursively();
                return false;
            }
        }
        return true;
    }
    if (QFile::copy(source, destination)) {
        return true;
    }
    *errorText = QObject::tr("Could not copy %1. Check that you have permission and enough disk space.")
                     .arg(sourceInfo.fileName());
    return false;
}

QString uniqueCopyPath(const QString &directory, const QFileInfo &source)
{
    const QString suffix = source.suffix();
    const QString stem = suffix.isEmpty() ? source.fileName() : source.completeBaseName();
    const QString extension = suffix.isEmpty() ? QString() : QStringLiteral(".") + suffix;
    for (int number = 1; number < 1000; ++number) {
        const QString label = number == 1
            ? QObject::tr("%1 (copy)%2").arg(stem, extension)
            : QObject::tr("%1 (copy %2)%3").arg(stem).arg(number).arg(extension);
        const QString candidate = QDir(directory).filePath(label);
        if (!QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

QPushButton *actionButton(const QString &label, QWidget *parent)
{
    auto *button = new QPushButton(label, parent);
    button->setFocusPolicy(Qt::StrongFocus);
    return button;
}
}

FileManagerWidget::FileManagerWidget(QWidget *parent)
    : QWidget(parent), currentPath_(QDir::homePath())
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(10);

    auto *title = new QLabel(tr("Files"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *description = new QLabel(
        tr("Browse folders and manage your files. Files moved to Trash can usually be restored from the system file manager."), this);
    description->setObjectName(QStringLiteral("intro"));
    description->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(description);

    auto *navigation = new QHBoxLayout;
    navigation->setSpacing(6);
    backButton_ = actionButton(tr("Back"), this);
    auto *upButton = actionButton(tr("Up"), this);
    auto *homeButton = actionButton(tr("Home"), this);
    backButton->setObjectName(QStringLiteral("fileAction"));
    upButton->setObjectName(QStringLiteral("fileAction"));
    homeButton->setObjectName(QStringLiteral("fileAction"));
    location_ = new QLineEdit(this);
    location_->setObjectName(QStringLiteral("fileLocation"));
    location_->setClearButtonEnabled(true);
    auto *goButton = actionButton(tr("Go"), this);
    goButton->setObjectName(QStringLiteral("primaryAction"));
    navigation->addWidget(backButton_);
    navigation->addWidget(upButton);
    navigation->addWidget(homeButton);
    navigation->addWidget(location_, 1);
    navigation->addWidget(goButton);
    layout->addLayout(navigation);

    auto *actions = new QHBoxLayout;
    actions->setSpacing(6);
    auto *newFolderButton = actionButton(tr("New folder"), this);
    auto *renameButton = actionButton(tr("Rename"), this);
    auto *copyButton = actionButton(tr("Copy"), this);
    auto *cutButton = actionButton(tr("Cut"), this);
    auto *pasteButton = actionButton(tr("Paste"), this);
    auto *shortcutButton = actionButton(tr("Create shortcut"), this);
    auto *trashButton = actionButton(tr("Move to Trash"), this);
    for (QPushButton *button : {newFolderButton, renameButton, copyButton, cutButton,
                                pasteButton, shortcutButton, trashButton}) {
        button->setObjectName(QStringLiteral("fileAction"));
    }
    actions->addWidget(newFolderButton);
    actions->addWidget(renameButton);
    actions->addWidget(copyButton);
    actions->addWidget(cutButton);
    actions->addWidget(pasteButton);
    layout->addLayout(actions);
    auto *secondaryActions = new QHBoxLayout;
    secondaryActions->setSpacing(6);
    secondaryActions->addWidget(shortcutButton);
    secondaryActions->addWidget(trashButton);
    secondaryActions->addStretch(1);
    layout->addLayout(secondaryActions);

    model_ = new QFileSystemModel(this);
    model_->setReadOnly(false);
    model_->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden);
    model_->setRootPath(currentPath_);

    view_ = new QTreeView(this);
    view_->setObjectName(QStringLiteral("fileView"));
    view_->setModel(model_);
    view_->setRootIndex(model_->index(currentPath_));
    view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view_->setSortingEnabled(true);
    view_->setDragEnabled(true);
    view_->setAcceptDrops(true);
    view_->setDropIndicatorShown(true);
    view_->setDragDropMode(QAbstractItemView::DragDrop);
    view_->setDefaultDropAction(Qt::MoveAction);
    view_->setContextMenuPolicy(Qt::CustomContextMenu);
    view_->header()->setStretchLastSection(false);
    view_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    view_->hideColumn(2);
    view_->hideColumn(3);
    layout->addWidget(view_, 1);

    status_ = new QLabel(this);
    status_->setObjectName(QStringLiteral("fileStatus"));
    layout->addWidget(status_);

    connect(backButton_, &QPushButton::clicked, this, &FileManagerWidget::navigateBack);
    connect(upButton, &QPushButton::clicked, this, [this]() {
        navigateTo(QDir(currentPath_).filePath(QStringLiteral("..")));
    });
    connect(homeButton, &QPushButton::clicked, this, [this]() { navigateTo(QDir::homePath()); });
    connect(goButton, &QPushButton::clicked, this, [this]() { navigateTo(location_->text()); });
    connect(location_, &QLineEdit::returnPressed, this, [this]() { navigateTo(location_->text()); });
    connect(newFolderButton, &QPushButton::clicked, this, &FileManagerWidget::createFolder);
    connect(renameButton, &QPushButton::clicked, this, &FileManagerWidget::renameSelected);
    connect(copyButton, &QPushButton::clicked, this, [this]() { copySelected(false); });
    connect(cutButton, &QPushButton::clicked, this, [this]() { copySelected(true); });
    connect(pasteButton, &QPushButton::clicked, this, &FileManagerWidget::pasteClipboard);
    connect(shortcutButton, &QPushButton::clicked, this, &FileManagerWidget::createShortcut);
    connect(trashButton, &QPushButton::clicked, this, &FileManagerWidget::moveToTrash);
    connect(view_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() { updateSelectionStatus(); });
    connect(view_, &QTreeView::customContextMenuRequested,
            this, &FileManagerWidget::showContextMenu);
    connect(view_, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        const QFileInfo info = model_->fileInfo(index);
        if (info.isDir()) {
            navigateTo(info.filePath());
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath()));
        }
    });

    auto addShortcut = [this](const QKeySequence &sequence, auto action) {
        auto *shortcut = new QShortcut(sequence, this);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, &QShortcut::activated, this, action);
    };
    addShortcut(QKeySequence::Copy, [this]() { copySelected(false); });
    addShortcut(QKeySequence::Cut, [this]() { copySelected(true); });
    addShortcut(QKeySequence::Paste, [this]() { pasteClipboard(); });
    addShortcut(QKeySequence(Qt::Key_F2), [this]() { renameSelected(); });
    addShortcut(QKeySequence(Qt::Key_Delete), [this]() { moveToTrash(); });
    addShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), [this]() {
        location_->setFocus();
        location_->selectAll();
    });
    addShortcut(QKeySequence(Qt::ALT | Qt::Key_Up), [this]() {
        navigateTo(QDir(currentPath_).filePath(QStringLiteral("..")));
    });

    location_->setText(currentPath_);
    visitedPaths_.append(currentPath_);
    visitedPathIndex_ = 0;
    backButton_->setEnabled(false);
    updateSelectionStatus();
}

QStringList FileManagerWidget::selectedPaths() const
{
    QStringList paths;
    const QModelIndexList rows = view_->selectionModel()->selectedRows(0);
    for (const QModelIndex &row : rows) {
        paths.push_back(model_->filePath(row));
    }
    return paths;
}

void FileManagerWidget::navigateTo(const QString &path, bool remember)
{
    QString expanded = QDir::fromNativeSeparators(path.trimmed());
    if (expanded == QStringLiteral("~")) {
        expanded = QDir::homePath();
    } else if (expanded.startsWith(QStringLiteral("~/"))) {
        expanded = QDir(QDir::homePath()).filePath(expanded.mid(2));
    }
    if (QDir::isRelativePath(expanded)) {
        expanded = QDir(currentPath_).absoluteFilePath(expanded);
    }
    expanded = QDir::cleanPath(expanded);
    const QFileInfo info(expanded);
    if (!info.exists() || !info.isDir() || !info.isReadable()) {
        QMessageBox::warning(this, tr("Folder unavailable"),
                             tr("LinWin can’t open this folder. Check the path and your access permissions."));
        return;
    }
    currentPath_ = info.canonicalFilePath().isEmpty() ? info.absoluteFilePath() : info.canonicalFilePath();
    if (remember && (visitedPathIndex_ < 0 || visitedPaths_.at(visitedPathIndex_) != currentPath_)) {
        while (visitedPaths_.size() > visitedPathIndex_ + 1) {
            visitedPaths_.removeLast();
        }
        visitedPaths_.append(currentPath_);
        visitedPathIndex_ = visitedPaths_.size() - 1;
    }
    location_->setText(currentPath_);
    view_->setRootIndex(model_->index(currentPath_));
    backButton_->setEnabled(visitedPathIndex_ > 0);
    updateSelectionStatus();
}

void FileManagerWidget::navigateBack()
{
    if (visitedPathIndex_ <= 0) {
        return;
    }
    --visitedPathIndex_;
    navigateTo(visitedPaths_.at(visitedPathIndex_), false);
}

void FileManagerWidget::updateSelectionStatus()
{
    const QStringList selection = selectedPaths();
    if (selection.isEmpty()) {
        status_->setText(tr("%1").arg(QDir::toNativeSeparators(currentPath_)));
        return;
    }
    status_->setText(tr("%1 item(s) selected  ·  %2")
                         .arg(selection.size())
                         .arg(QDir::toNativeSeparators(currentPath_)));
}

void FileManagerWidget::createFolder()
{
    bool accepted = false;
    const QString name = QInputDialog::getText(this, tr("New folder"), tr("Folder name:"),
                                                QLineEdit::Normal, QString(), &accepted).trimmed();
    if (!accepted || name.isEmpty()) {
        return;
    }
    const QString path = QDir(currentPath_).filePath(name);
    if (!QDir().mkdir(path)) {
        QMessageBox::warning(this, tr("Couldn’t create folder"),
                             tr("A folder with that name may already exist, or you may not have permission."));
        return;
    }
    view_->setCurrentIndex(model_->index(path));
    updateSelectionStatus();
}

void FileManagerWidget::renameSelected()
{
    const QStringList selection = selectedPaths();
    if (selection.size() != 1) {
        status_->setText(tr("Select one item to rename."));
        return;
    }
    const QFileInfo source(selection.front());
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Rename item"));
    auto *layout = new QVBoxLayout(dialog);
    layout->addWidget(new QLabel(tr("Choose a new name:"), dialog));
    auto *nameField = new QLineEdit(source.fileName(), dialog);
    nameField->setObjectName(QStringLiteral("renameField"));
    layout->addWidget(nameField);
    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *cancel = new QPushButton(tr("Cancel"), dialog);
    auto *rename = new QPushButton(tr("Rename"), dialog);
    rename->setObjectName(QStringLiteral("primaryAction"));
    buttons->addWidget(cancel);
    buttons->addWidget(rename);
    layout->addLayout(buttons);
    connect(cancel, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(rename, &QPushButton::clicked, dialog, &QDialog::accept);
    const int extensionIndex = source.suffix().isEmpty()
        ? nameField->text().size()
        : qMax(0, nameField->text().size() - source.suffix().size() - 1);
    nameField->setSelection(0, extensionIndex);
    nameField->setFocus();
    if (dialog->exec() != QDialog::Accepted) {
        dialog->deleteLater();
        return;
    }
    const QString newName = nameField->text().trimmed();
    dialog->deleteLater();
    if (newName.isEmpty() || newName == source.fileName()) {
        return;
    }
    const QString destination = QDir(source.absolutePath()).filePath(newName);
    if (QFileInfo::exists(destination)) {
        QMessageBox::warning(this, tr("Name already in use"), tr("Choose a name that isn’t already in this folder."));
        return;
    }
    if (!QDir().rename(source.filePath(), destination)) {
        QMessageBox::warning(this, tr("Couldn’t rename item"),
                             tr("Check that you have permission to change this name."));
    }
}

void FileManagerWidget::copySelected(bool cut)
{
    const QStringList paths = selectedPaths();
    if (paths.isEmpty()) {
        status_->setText(tr("Select one or more items first."));
        return;
    }
    auto *mime = new QMimeData;
    QList<QUrl> urls;
    QByteArray gnomePayload(cut ? "cut\n" : "copy\n");
    for (const QString &path : paths) {
        const QUrl url = QUrl::fromLocalFile(path);
        urls.push_back(url);
        gnomePayload += url.toString(QUrl::FullyEncoded).toUtf8() + '\n';
    }
    mime->setUrls(urls);
    mime->setData(kGnomeFilesMime, gnomePayload);
    if (cut) {
        mime->setData(kLinWinCutMime, QByteArrayLiteral("1"));
    }
    QApplication::clipboard()->setMimeData(mime);
    status_->setText(cut ? tr("Ready to move %1 item(s). Choose a folder and paste.").arg(paths.size())
                         : tr("Copied %1 item(s). Choose a folder and paste.").arg(paths.size()));
}

void FileManagerWidget::pasteClipboard()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    const QList<QUrl> urls = mime->urls();
    if (urls.isEmpty()) {
        status_->setText(tr("The clipboard doesn’t contain files to paste."));
        return;
    }
    const bool cut = mime->hasFormat(kLinWinCutMime)
        || (mime->hasFormat(kGnomeFilesMime)
            && mime->data(kGnomeFilesMime).startsWith(QByteArrayLiteral("cut\n")));
    int completed = 0;
    QString failure;
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            failure = tr("Only local files can be pasted here.");
            break;
        }
        const QString sourcePath = url.toLocalFile();
        const QFileInfo source(sourcePath);
        if (!source.exists() && !source.isSymLink()) {
            failure = tr("One of the selected items no longer exists.");
            break;
        }
        const QString sourceDirectory = QDir::cleanPath(source.absoluteFilePath());
        const QString destinationDirectory = QDir::cleanPath(QDir(currentPath_).absolutePath());
        if (source.isDir()
            && (destinationDirectory == sourceDirectory
                || destinationDirectory.startsWith(sourceDirectory + QDir::separator()))) {
            failure = tr("A folder can’t be pasted into itself or one of its own folders.");
            break;
        }
        QString destination = QDir(currentPath_).filePath(source.fileName());
        if (QFileInfo::exists(destination)) {
            if (cut) {
                failure = tr("%1 already exists here. Rename or move it before pasting.").arg(source.fileName());
                break;
            }
            destination = uniqueCopyPath(currentPath_, source);
        }
        if (destination.isEmpty()) {
            failure = tr("Couldn’t choose a new name for the copied item.");
            break;
        }
        if (cut) {
            if (!QDir().rename(source.absoluteFilePath(), destination)) {
                failure = tr("Couldn’t move %1. Check that both folders allow this operation.").arg(source.fileName());
                break;
            }
        } else if (!copyPath(source.absoluteFilePath(), destination, &failure)) {
            break;
        }
        ++completed;
    }

    if (completed > 0 && cut) {
        QApplication::clipboard()->clear();
    }
    if (!failure.isEmpty()) {
        status_->setText(tr("Pasted %1 item(s). %2").arg(completed).arg(failure));
        QMessageBox::warning(this, tr("Some items weren’t pasted"), status_->text());
    } else {
        status_->setText(tr("Pasted %1 item(s) into this folder.").arg(completed));
    }
}

void FileManagerWidget::moveToTrash()
{
    const QStringList paths = selectedPaths();
    if (paths.isEmpty()) {
        status_->setText(tr("Select one or more items first."));
        return;
    }
    const auto answer = QMessageBox::question(
        this, tr("Move to Trash?"),
        tr("Move %1 selected item(s) to the system Trash? You can usually restore them from your desktop file manager.").arg(paths.size()),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (answer != QMessageBox::Yes) {
        return;
    }
    int moved = 0;
    QString failure;
    for (const QString &path : paths) {
        GFile *file = g_file_new_for_path(path.toUtf8().constData());
        GError *error = nullptr;
        const gboolean trashed = g_file_trash(file, nullptr, &error);
        g_object_unref(file);
        if (!trashed) {
            failure = error != nullptr ? QString::fromUtf8(error->message) : tr("The system Trash is unavailable.");
            if (error != nullptr) {
                g_error_free(error);
            }
            break;
        }
        ++moved;
    }
    if (!failure.isEmpty()) {
        status_->setText(tr("Moved %1 item(s) to Trash. %2").arg(moved).arg(failure));
        QMessageBox::warning(this, tr("Couldn’t move every item to Trash"), status_->text());
    } else {
        status_->setText(tr("Moved %1 item(s) to Trash.").arg(moved));
    }
}

void FileManagerWidget::createShortcut()
{
    const QStringList paths = selectedPaths();
    if (paths.size() != 1) {
        status_->setText(tr("Select one item to create a shortcut."));
        return;
    }
    const QFileInfo source(paths.front());
    const QString linkPath = QDir(currentPath_).filePath(tr("Link to %1").arg(source.fileName()));
    if (QFileInfo::exists(linkPath)) {
        QMessageBox::warning(this, tr("Shortcut already exists"),
                             tr("A shortcut with that name already exists in this folder."));
        return;
    }
    const QString target = source.canonicalFilePath().isEmpty() ? source.absoluteFilePath() : source.canonicalFilePath();
    if (!QFile::link(target, linkPath)) {
        QMessageBox::warning(this, tr("Couldn’t create shortcut"),
                             tr("Check that you have permission to create a link in this folder."));
        return;
    }
    status_->setText(tr("Created shortcut “%1”.").arg(QFileInfo(linkPath).fileName()));
}

void FileManagerWidget::showContextMenu(const QPoint &position)
{
    const QModelIndex index = view_->indexAt(position);
    if (index.isValid() && !view_->selectionModel()->isSelected(index)) {
        view_->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    QMenu menu(this);
    QAction *openAction = menu.addAction(tr("Open"));
    QAction *newFolderAction = menu.addAction(tr("New folder"));
    menu.addSeparator();
    QAction *cutAction = menu.addAction(tr("Cut"));
    QAction *copyAction = menu.addAction(tr("Copy"));
    QAction *pasteAction = menu.addAction(tr("Paste"));
    QAction *renameAction = menu.addAction(tr("Rename"));
    QAction *shortcutAction = menu.addAction(tr("Create shortcut"));
    QAction *trashAction = menu.addAction(tr("Move to Trash"));
    openAction->setEnabled(index.isValid());
    renameAction->setEnabled(selectedPaths().size() == 1);
    shortcutAction->setEnabled(selectedPaths().size() == 1);
    cutAction->setEnabled(!selectedPaths().isEmpty());
    copyAction->setEnabled(!selectedPaths().isEmpty());
    pasteAction->setEnabled(!QApplication::clipboard()->mimeData()->urls().isEmpty());
    trashAction->setEnabled(!selectedPaths().isEmpty());

    QAction *chosen = menu.exec(view_->viewport()->mapToGlobal(position));
    if (chosen == openAction && index.isValid()) {
        const QFileInfo info = model_->fileInfo(index);
        if (info.isDir()) {
            navigateTo(info.filePath());
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath()));
        }
    } else if (chosen == newFolderAction) {
        createFolder();
    } else if (chosen == cutAction) {
        copySelected(true);
    } else if (chosen == copyAction) {
        copySelected(false);
    } else if (chosen == pasteAction) {
        pasteClipboard();
    } else if (chosen == renameAction) {
        renameSelected();
    } else if (chosen == shortcutAction) {
        createShortcut();
    } else if (chosen == trashAction) {
        moveToTrash();
    }
}
