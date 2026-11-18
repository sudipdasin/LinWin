// SPDX-License-Identifier: GPL-3.0-or-later
#include "setup_wizard.h"

#include <QApplication>
#include <QLabel>
#include <QWizardPage>

#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    SetupWizard setup;
    if (setup.pageIds().size() != 2) {
        std::cerr << "Setup must provide welcome and system review steps.\n";
        return 1;
    }
    const auto *compatibility = setup.findChild<QLabel *>(QStringLiteral("setupCompatibility"));
    const auto *review = setup.findChild<QLabel *>(QStringLiteral("setupReview"));
    if (compatibility == nullptr || review == nullptr || !review->text().contains(QStringLiteral("/usr/share/xsessions"))) {
        std::cerr << "Setup review must explain the exact session integration.\n";
        return 1;
    }

    SetupWizard uninstall(true);
    const auto *uninstallReview = uninstall.findChild<QLabel *>(QStringLiteral("setupReview"));
    if (uninstallReview == nullptr || !uninstallReview->text().contains(QStringLiteral("package manager"))) {
        std::cerr << "Uninstall review must explain package-manager removal.\n";
        return 1;
    }
    return 0;
}
