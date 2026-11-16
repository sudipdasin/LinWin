// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "system_profile.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace {
QFrame *makeCard(const QString &title, const QString &body, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("card"));
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    auto *heading = new QLabel(title, card);
    heading->setObjectName(QStringLiteral("cardTitle"));
    heading->setWordWrap(true);
    auto *description = new QLabel(body, card);
    description->setObjectName(QStringLiteral("cardBody"));
    description->setWordWrap(true);
    layout->addWidget(heading);
    layout->addWidget(description);
    layout->addStretch();
    return card;
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("LinWin — Linux usability tools"));
    setMinimumSize(520, 420);
    resize(1040, 700);

    const SystemProfile profile = SystemProfile::detect();
    auto *root = new QWidget(this);
    auto *rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    sidebar_ = new QLabel(root);
    sidebar_->setObjectName(QStringLiteral("sidebar"));
    sidebar_->setText(QStringLiteral("LINWIN\n\nOverview\nRoadmap\nSystem info"));
    sidebar_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    sidebar_->setMargin(22);
    sidebar_->setMinimumWidth(176);
    sidebar_->setMaximumWidth(220);
    sidebar_->setTextInteractionFlags(Qt::NoTextInteraction);
    rootLayout->addWidget(sidebar_);

    auto *scroll = new QScrollArea(root);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scroll);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(18);

    auto *eyebrow = new QLabel(QStringLiteral("CROSS-DISTRO DESKTOP TOOLS"), content);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    auto *title = new QLabel(QStringLiteral("Make Linux feel at home."), content);
    title->setObjectName(QStringLiteral("pageTitle"));
    title->setWordWrap(true);
    auto *intro = new QLabel(
        QStringLiteral("LinWin is an open-source project for making everyday Linux desktop tasks simpler across distributions and desktop environments."), content);
    intro->setObjectName(QStringLiteral("intro"));
    intro->setWordWrap(true);
    layout->addWidget(eyebrow);
    layout->addWidget(title);
    layout->addWidget(intro);

    auto *systemHeading = new QLabel(QStringLiteral("This system"), content);
    systemHeading->setObjectName(QStringLiteral("sectionTitle"));
    layout->addWidget(systemHeading);
    auto *systemGrid = new QGridLayout;
    systemGrid->setSpacing(12);
    systemGrid->addWidget(makeCard(QStringLiteral("Distribution"), profile.distribution, content), 0, 0);
    systemGrid->addWidget(makeCard(QStringLiteral("Desktop"), profile.desktopEnvironment, content), 0, 1);
    systemGrid->addWidget(makeCard(QStringLiteral("Session"), profile.sessionType, content), 1, 0);
    systemGrid->addWidget(makeCard(QStringLiteral("Kernel"), profile.kernel, content), 1, 1);
    layout->addLayout(systemGrid);

    auto *statusHeading = new QLabel(QStringLiteral("Project status"), content);
    statusHeading->setObjectName(QStringLiteral("sectionTitle"));
    auto *status = new QLabel(QStringLiteral("Early development — shared core scaffold"), content);
    status->setObjectName(QStringLiteral("statusBadge"));
    auto *statusDescription = new QLabel(
        QStringLiteral("This first build detects the current Linux environment. Clipboard, file operations, Bluetooth, and desktop-shell integrations are planned and are not yet available."), content);
    statusDescription->setObjectName(QStringLiteral("intro"));
    statusDescription->setWordWrap(true);
    layout->addWidget(statusHeading);
    layout->addWidget(status);
    layout->addWidget(statusDescription);

    auto *compatibilityHeading = new QLabel(QStringLiteral("Designed for choice"), content);
    compatibilityHeading->setObjectName(QStringLiteral("sectionTitle"));
    auto *compatibility = new QLabel(
        QStringLiteral("The plan is to support Linux Mint Cinnamon, Xfce, and MATE, plus KDE Plasma and additional desktops through dedicated integrations."), content);
    compatibility->setObjectName(QStringLiteral("intro"));
    compatibility->setWordWrap(true);
    layout->addWidget(compatibilityHeading);
    layout->addWidget(compatibility);
    layout->addStretch();

    scroll->setWidget(content);
    rootLayout->addWidget(scroll, 1);
    setCentralWidget(root);

    setStyleSheet(QStringLiteral(
        "QMainWindow, QWidget { background: #f5f7fb; color: #172033; font-family: 'Sans Serif'; }"
        "#sidebar { background: #101a2b; color: #f4f7ff; font-size: 15px; font-weight: 600; }"
        "#eyebrow { color: #3568d4; font-size: 11px; font-weight: 700; letter-spacing: 1px; }"
        "#pageTitle { color: #111a2b; font-size: 30px; font-weight: 700; }"
        "#sectionTitle { color: #172033; font-size: 18px; font-weight: 650; margin-top: 6px; }"
        "#intro { color: #526078; font-size: 14px; }"
        "#card { background: white; border: 1px solid #e0e6ef; border-radius: 12px; min-height: 78px; }"
        "#cardTitle { color: #526078; font-size: 12px; font-weight: 600; }"
        "#cardBody { color: #172033; font-size: 16px; font-weight: 600; }"
        "#statusBadge { color: #714c06; background: #fff0c2; border-radius: 8px; padding: 8px 12px; font-weight: 600; }"));
    updateResponsiveLayout(width());
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateResponsiveLayout(event->size().width());
}

void MainWindow::updateResponsiveLayout(int width)
{
    if (sidebar_ != nullptr) {
        sidebar_->setVisible(width >= 760);
    }
}
