// SPDX-License-Identifier: GPL-3.0-or-later
#include "clipboard_history_widget.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCoreApplication>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QMetaObject>

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
    auto *captureToggle = widget.findChild<QCheckBox *>(QStringLiteral("captureClipboard"));
    captureToggle->setChecked(true);
    QApplication::clipboard()->setText(QStringLiteral("first clip"));
    application.processEvents();
    const int savedCount = history->count();
    if (savedCount != 1) {
        std::cerr << "Enabled clipboard history should capture a new text clip\n";
        return 1;
    }
    widget.setComponentEnabled(false);
    if (widget.isEnabled()) {
        std::cerr << "Disabled clipboard component should not remain interactive\n";
        return 1;
    }
    QApplication::clipboard()->setText(QStringLiteral("must not be captured while disabled"));
    QMetaObject::invokeMethod(&widget, "captureClipboardText", Qt::DirectConnection);
    if (history->count() != savedCount) {
        std::cerr << "Disabled clipboard component must pause capture\n";
        return 1;
    }
    widget.setComponentEnabled(true);
    if (!widget.isEnabled()) {
        std::cerr << "Re-enabled clipboard component should become available\n";
        return 1;
    }
    settings.clear();
    return 0;
}
