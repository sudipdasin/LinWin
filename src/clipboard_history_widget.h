// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QStringList>

class QCheckBox;
class QClipboard;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class ClipboardHistoryWidget final : public QWidget {
    Q_OBJECT
public:
    explicit ClipboardHistoryWidget(QWidget *parent = nullptr);

private slots:
    void captureClipboardText();
    void copySelectedEntry();
    void removeSelectedEntry();
    void clearHistory();
    void filterEntries(const QString &query);

private:
    void saveHistory() const;
    void refreshList();
    void updateCaptureState(bool enabled);

    QClipboard *clipboard_ = nullptr;
    QCheckBox *captureCheckBox_ = nullptr;
    QLineEdit *searchField_ = nullptr;
    QListWidget *historyList_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QStringList history_;
};
