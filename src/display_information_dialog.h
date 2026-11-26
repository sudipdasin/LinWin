// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QTableWidget>
#include <QVBoxLayout>

class DisplayInformationDialog final : public QDialog {
public:
    explicit DisplayInformationDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setObjectName(QStringLiteral("linwinDisplayInformation"));
        setWindowTitle(tr("Display information"));
        resize(760, 340);
        auto *layout = new QVBoxLayout(this);
        auto *description = new QLabel(tr("Display properties reported by your current desktop session. Resolution is in logical pixels; color depth does not establish HDR support."), this);
        description->setWordWrap(true);
        layout->addWidget(description);
        table_ = new QTableWidget(this);
        table_->setObjectName(QStringLiteral("linwinDisplayTable"));
        table_->setColumnCount(6);
        table_->setHorizontalHeaderLabels({tr("Display"), tr("Logical size"), tr("Scale"), tr("Refresh rate"), tr("Logical DPI"), tr("Depth")});
        table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table_->setAlternatingRowColors(true);
        table_->verticalHeader()->hide();
        table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table_);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
        auto *refresh = buttons->addButton(tr("Refresh"), QDialogButtonBox::ActionRole);
        connect(refresh, &QPushButton::clicked, this, [this]() { populate(); });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(buttons);
        auto *app = qobject_cast<QGuiApplication *>(QGuiApplication::instance());
        if (app) {
            connect(app, &QGuiApplication::screenAdded, this, [this](QScreen *screen) { observe(screen); populate(); });
            connect(app, &QGuiApplication::screenRemoved, this, [this](QScreen *) { populate(); });
        }
        for (QScreen *screen : QGuiApplication::screens()) observe(screen);
        populate();
    }
private:
    void observe(QScreen *screen)
    {
        if (!screen) return;
        connect(screen, &QScreen::geometryChanged, this, [this]() { populate(); });
        connect(screen, &QScreen::logicalDotsPerInchChanged, this, [this]() { populate(); });
        connect(screen, &QScreen::refreshRateChanged, this, [this]() { populate(); });
    }
    void populate()
    {
        const auto screens = QGuiApplication::screens();
        table_->setRowCount(static_cast<int>(screens.size()));
        for (qsizetype i = 0; i < screens.size(); ++i) {
            const auto *screen = screens.at(i);
            const int row = static_cast<int>(i);
            const auto size = screen->size();
            QString name = screen->name().isEmpty() ? tr("Display %1").arg(i + 1) : screen->name();
            if (screen == QGuiApplication::primaryScreen()) name += tr(" (primary)");
            const QStringList cells{
                name, tr("%1 × %2").arg(size.width()).arg(size.height()),
                tr("%1%").arg(qRound(screen->devicePixelRatio() * 100)),
                screen->refreshRate() > 0 ? tr("%1 Hz").arg(screen->refreshRate(), 0, 'f', 1) : tr("Unknown"),
                QString::number(screen->logicalDotsPerInch(), 'f', 1),
                tr("%1 bit").arg(screen->depth())
            };
            for (int col = 0; col < cells.size(); ++col)
                table_->setItem(row, col, new QTableWidgetItem(cells.at(col)));
        }
    }
    QTableWidget *table_ = nullptr;
};
