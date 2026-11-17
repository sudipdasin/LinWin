// SPDX-License-Identifier: GPL-3.0-or-later
#include "clipboard_history_widget.h"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>

#include <iostream>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("LinWinTest"));
    QCoreApplication::setApplicationName(QStringLiteral("ClipboardCenterTest"));
    QSettings settings;
    settings.clear();

    ClipboardHistoryWidget widget;
    const QStringList required{
        QStringLiteral("captureClipboard"), QStringLiteral("middleClickPaste"),
        QStringLiteral("clipboardSearch"), QStringLiteral("clipboardEntries"),
        QStringLiteral("pinClipboardItem")
    };
    for (const QString &name : required) {
        if (widget.findChild<QWidget *>(name) == nullptr) {
            std::cerr << "Missing clipboard control: " << name.toStdString() << '\n';
            return 1;
        }
    }
    const auto *capture = widget.findChild<QCheckBox *>(QStringLiteral("captureClipboard"));
    const auto *middleClick = widget.findChild<QCheckBox *>(QStringLiteral("middleClickPaste"));
    if (capture == nullptr || capture->isChecked() || middleClick == nullptr || middleClick->isChecked()) {
        std::cerr << "Clipboard capture and middle-click paste must start off\n";
        return 1;
    }
    const auto *history = widget.findChild<QListWidget *>(QStringLiteral("clipboardEntries"));
    if (history == nullptr || history->count() != 0) {
        std::cerr << "Clipboard history should start empty\n";
        return 1;
    }
    return 0;
}
