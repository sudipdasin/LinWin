// SPDX-License-Identifier: GPL-3.0-or-later
#include "main_window.h"
#include "system_profile.h"
#include "clipboard_history_widget.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTabWidget>
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
    setWindowTitle(QStringLiteral("LinWin — Linux desktop shell"));
    setMinimumSize(380, 420);
    resize(1040, 700);

    const SystemProfile profile = SystemProfile::detect();
    pages_ = new QTabWidget(this);
    pages_->setDocumentMode(true);
    pages_->setObjectName(QStringLiteral("mainPages"));

    auto *scroll = new QScrollArea(pages_);
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
        QStringLiteral("A lightweight shell replacement for Linux desktops, built around familiar workflows and optional components."), content);
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
    auto *status = new QLabel(QStringLiteral("Early development — shell prototype"), content);
    status->setObjectName(QStringLiteral("statusBadge"));
    auto *statusDescription = new QLabel(
        QStringLiteral("This build is a prototype, not yet a full shell replacement. The clipboard component is optional; the taskbar, launcher, hardware controls, session integration, and installation wizard are in development."), content);
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
    pages_->addTab(scroll, tr("Overview"));
    pages_->addTab(new ClipboardHistoryWidget(pages_), tr("Clipboard"));
    setCentralWidget(pages_);

    setStyleSheet(QStringLiteral(
        "QMainWindow, QWidget { background: #f5f7fb; color: #172033; font-family: 'Sans Serif'; }"
        "QTabWidget::pane { border: 0; }"
        "QTabBar::tab { padding: 10px 18px; color: #526078; }"
        "QTabBar::tab:selected { color: #2456bd; border-bottom: 2px solid #3568d4; font-weight: 700; }"
        "#eyebrow { color: #3568d4; font-size: 11px; font-weight: 700; letter-spacing: 1px; }"
        "#pageTitle { color: #111a2b; font-size: 30px; font-weight: 700; }"
        "#sectionTitle { color: #172033; font-size: 18px; font-weight: 650; margin-top: 6px; }"
        "#intro { color: #526078; font-size: 14px; }"
        "#card { background: white; border: 1px solid #e0e6ef; border-radius: 12px; min-height: 78px; }"
        "#cardTitle { color: #526078; font-size: 12px; font-weight: 600; }"
        "#cardBody { color: #172033; font-size: 16px; font-weight: 600; }"
        "#statusBadge { color: #714c06; background: #fff0c2; border-radius: 8px; padding: 8px 12px; font-weight: 600; }"
        "#privacyNote { color: #654d17; background: #fff7dc; border: 1px solid #f0dda1; border-radius: 8px; padding: 10px 12px; }"
        "#clipboardEntries { background: white; border: 1px solid #e0e6ef; border-radius: 8px; padding: 4px; }"
        "#primaryAction { background: #3568d4; color: white; border: 0; border-radius: 6px; padding: 8px 14px; font-weight: 600; }"));
}
