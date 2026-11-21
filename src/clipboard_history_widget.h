// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QStringList>

class QCheckBox;
class QClipboard;
class QDialog;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QEvent;
class QObject;

class ClipboardHistoryWidget final : public QWidget {
    Q_OBJECT
public:
    explicit ClipboardHistoryWidget(QWidget *parent = nullptr);
    void focusSearch();
    void setComponentEnabled(bool enabled);
    void showQuickPicker(QWidget *anchor = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void captureClipboardText();
    void copySelectedEntry();
    void removeSelectedEntry();
    void clearHistory();
    void toggleSelectedPin();
    void copyQuickPickEntry(QListWidgetItem *item);
    void updateMiddleClickPreference(bool enabled);
    void filterEntries(const QString &query);

private:
    void saveHistory() const;
    void refreshList();
    void updateCaptureState(bool enabled);
    void updatePinButtonLabel();

    QClipboard *clipboard_ = nullptr;
    QCheckBox *captureCheckBox_ = nullptr;
    QCheckBox *middleClickPasteCheckBox_ = nullptr;
    QPushButton *pinButton_ = nullptr;
    QLineEdit *searchField_ = nullptr;
    QListWidget *historyList_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QStringList history_;
    QStringList pinned_;
    bool componentEnabled_ = true;
    QDialog *quickPicker_ = nullptr;
};
