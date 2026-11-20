// SPDX-License-Identifier: GPL-3.0-or-later
#include "setup_wizard.h"

#include "system_profile.h"
#include "xfce_dependency_plan.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <QWizardPage>

#include <unistd.h>

#ifndef LINWIN_SESSION_HELPER_PATH
#define LINWIN_SESSION_HELPER_PATH "/usr/libexec/linwin-session-helper"
#endif

namespace {
QString helperPath()
{
    const QString installed = QString::fromUtf8(LINWIN_SESSION_HELPER_PATH);
    if (QFileInfo(installed).isExecutable())
        return installed;
    const QString sibling = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("linwin-session-helper"));
    return QFileInfo(sibling).isExecutable() ? sibling : QString();
}

bool hasXfceSessionTools(QStringList *missing)
{
    const QStringList required{QStringLiteral("xfwm4"), QStringLiteral("xfsettingsd"), QStringLiteral("xfdesktop")};
    for (const QString &tool : required) {
        if (QStandardPaths::findExecutable(tool).isEmpty())
            missing->append(tool);
    }
    return missing->isEmpty();
}

QWizardPage *makePage(const QString &title, const QString &subtitle, QWidget *body, QWidget *parent)
{
    auto *page = new QWizardPage(parent);
    page->setTitle(title);
    page->setSubTitle(subtitle);
    auto *layout = new QVBoxLayout(page);
    layout->addWidget(body);
    layout->addStretch();
    return page;
}
}

SetupWizard::SetupWizard(bool uninstallMode, QWidget *parent)
    : QWizard(parent), uninstallMode_(uninstallMode), helperProcess_(new QProcess(this))
{
    setWindowTitle(uninstallMode_ ? tr("Remove LinWin") : tr("LinWin Setup"));
    setWizardStyle(QWizard::ModernStyle);
    setMinimumSize(640, 500);
    setOption(QWizard::HaveCustomButton1, true);
    setButtonText(QWizard::CustomButton1, uninstallMode_ ? tr("Uninstall LinWin…") : tr("Install LinWin session…"));
    setButtonLayout({QWizard::Stretch, QWizard::BackButton, QWizard::NextButton,
        QWizard::CustomButton1, QWizard::FinishButton, QWizard::CancelButton});

    auto *welcome = new QLabel(
        tr("LinWin can start as a selectable X11 login session while leaving your installed Linux distribution, kernel, drivers, applications, and existing desktop in place.\n\n"
           "This first session integration uses the XFCE window manager and desktop. Your current desktop remains available from the login screen as a recovery option."));
    welcome->setWordWrap(true);
    auto *welcomePage = makePage(tr("Welcome to LinWin Setup"), tr("Review support and system changes before continuing."), welcome, this);
    addPage(welcomePage);

    auto *reviewBody = new QWidget;
    auto *reviewLayout = new QVBoxLayout(reviewBody);
    compatibility_ = new QLabel(reviewBody);
    compatibility_->setObjectName(QStringLiteral("setupCompatibility"));
    compatibility_->setWordWrap(true);
    review_ = new QLabel(reviewBody);
    review_->setObjectName(QStringLiteral("setupReview"));
    review_->setWordWrap(true);
    result_ = new QLabel(reviewBody);
    result_->setObjectName(QStringLiteral("setupResult"));
    result_->setWordWrap(true);
    result_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    reviewLayout->addWidget(compatibility_);
    reviewLayout->addWidget(review_);
    reviewLayout->addWidget(result_);
    auto *reviewPage = makePage(tr("System check and review"), tr("Review package changes and the session entry before authorizing them."), reviewBody, this);
    addPage(reviewPage);

    connect(this, &QWizard::currentIdChanged, this, [this]() { updateReview(); });
    connect(this, &QWizard::customButtonClicked, this, [this](int button) {
        if (button == QWizard::CustomButton1)
            runAction(uninstallMode_);
    });
    connect(helperProcess_, &QProcess::readyReadStandardError, this, [this]() {
        const QString output = QString::fromLocal8Bit(helperProcess_->readAllStandardError()).trimmed();
        if (!output.isEmpty()) {
            helperOutput_ += output + QLatin1Char('\n');
            result_->setText(helperOutput_.trimmed());
        }
    });
    connect(helperProcess_, &QProcess::readyReadStandardOutput, this, [this]() {
        QStringList lines = QString::fromLocal8Bit(helperProcess_->readAllStandardOutput())
            .split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
        if (!lines.isEmpty() && pendingAction_ == QStringLiteral("install-xfce-deps"))
            result_->setText(tr("Installing XFCE session components…\n%1").arg(lines.constLast().trimmed().left(240)));
    });
    connect(helperProcess_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this, &SetupWizard::finishAction);
    connect(helperProcess_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            button(QWizard::BackButton)->setEnabled(true);
            button(QWizard::NextButton)->setEnabled(true);
            button(QWizard::CancelButton)->setEnabled(true);
            button(QWizard::CustomButton1)->setEnabled(true);
            result_->setText(tr("Could not start the authorization helper. Check that polkit and its desktop authentication agent are installed."));
        }
    });
    updateReview();
}

void SetupWizard::updateReview()
{
    const SystemProfile profile = SystemProfile::detect();
    const QString sessionType = profile.sessionType;
    QStringList missing;
    const bool xfceReady = hasXfceSessionTools(&missing);
    const bool hasPkexec = !QStandardPaths::findExecutable(QStringLiteral("pkexec")).isEmpty();
    const bool helperReady = !helperPath().isEmpty();
    QFile osRelease(QStringLiteral("/etc/os-release"));
    const QString osReleaseContents = osRelease.open(QIODevice::ReadOnly)
        ? QString::fromUtf8(osRelease.readAll()) : QString();
    const PackageManager manager = packageManagerForOsRelease(osReleaseContents);
    const QString managerProgram = packageManagerProgram(manager);
    const bool packageManagerReady = !managerProgram.isEmpty()
        && !QStandardPaths::findExecutable(managerProgram).isEmpty();
    const bool installDependencies = !uninstallMode_ && !xfceReady;
    button(QWizard::CustomButton1)->setText(uninstallMode_ ? tr("Uninstall LinWin…")
        : installDependencies ? tr("Install XFCE components…") : tr("Install LinWin session…"));

    compatibility_->setText(tr("<b>Detected system</b><br>%1<br>Desktop: %2<br>Session now: %3<br>Kernel: %4")
        .arg(profile.distribution.toHtmlEscaped())
        .arg(profile.desktopEnvironment.toHtmlEscaped())
        .arg(sessionType.toHtmlEscaped())
        .arg(profile.kernel.toHtmlEscaped()));

    if (uninstallMode_) {
        review_->setText(tr("<b>Uninstall plan</b><br>1. Remove LinWin's selectable login-session entry.<br>2. Ask the system authorization service to remove the LinWin package with the detected distro package manager.<br><br>The current session stays open until you log out. Select your previous desktop from the login screen afterward."));
    } else {
        const QString nextStep = installDependencies
            ? tr("<br><br><b>Next step</b><br>Install only the detected distribution's XFCE session packages. Review the exact package list before authorizing installation.")
            : tr("<br><br><b>Next step</b><br>Register LinWin as an X11 login session in <code>/usr/share/xsessions</code>. Your current desktop remains available as a recovery option.");
        review_->setText(tr("<b>Setup plan</b><br>LinWin and the XFCE window manager/desktop run as your normal user. No kernel, driver, or unrelated service changes are made.%1").arg(nextStep));
    }

    QString reason;
    if (!hasPkexec)
        reason = tr("The system authorization tool (pkexec) is unavailable. Install polkit and a desktop authentication agent.");
    else if (!helperReady)
        reason = tr("The LinWin session helper is not installed. Install LinWin from its distro package before running setup.");
    else if (geteuid() == 0)
        reason = tr("For safety, run this wizard from your normal desktop account, not as root.");
    else if (!uninstallMode_ && !xfceReady) {
        if (packageManagerReady) {
            const QStringList packages = xfceDependencyPackages(manager);
            reason = tr("Missing XFCE session tools: %1. The wizard can install these packages with %2: %3.")
                .arg(missing.join(QStringLiteral(", ")), packageManagerProgram(manager), packages.join(QStringLiteral(", ")));
        } else {
            reason = tr("Missing XFCE session tools: %1. Automatic installation is unavailable for this distribution; use its graphical software manager to add XFCE session components.")
                .arg(missing.join(QStringLiteral(", ")));
        }
    }
    else
        reason = tr("Ready. The next step opens your desktop's authorization prompt.");
    compatibility_->setText(compatibility_->text() + QStringLiteral("<br><br><b>Readiness:</b> ") + reason.toHtmlEscaped());

    button(QWizard::CustomButton1)->setEnabled(currentId() == 1 && helperReady && hasPkexec && geteuid() != 0
        && (uninstallMode_ || xfceReady || packageManagerReady) && helperProcess_->state() == QProcess::NotRunning);
}

void SetupWizard::runAction(bool uninstall)
{
    if (helperProcess_->state() != QProcess::NotRunning)
        return;
    QStringList missing;
    const bool needsDependencies = !uninstall && !hasXfceSessionTools(&missing);
    QFile osRelease(QStringLiteral("/etc/os-release"));
    const PackageManager manager = osRelease.open(QIODevice::ReadOnly)
        ? packageManagerForOsRelease(QString::fromUtf8(osRelease.readAll())) : PackageManager::Unsupported;
    const QString action = uninstall ? tr("Remove the LinWin session and uninstall its package?")
        : needsDependencies ? tr("Install the XFCE session components needed by LinWin?")
                            : tr("Install the LinWin X11 login-session entry?");
    const QString impact = uninstall
        ? tr("This removes the LinWin package using the distro package manager. Your current desktop remains available.")
        : needsDependencies
            ? tr("Packages to install: %1. The native package manager may add their required dependencies. No kernel or driver packages are requested. You will see the system authorization prompt.")
                .arg(xfceDependencyPackages(manager).join(QStringLiteral(", ")))
            : tr("Only the login-session entry is added. No files in your existing desktop session are replaced.");
    if (QMessageBox::question(this, tr("Confirm LinWin change"), action + QStringLiteral("\n\n") + impact,
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    const QString helper = helperPath();
    if (helper.isEmpty()) {
        result_->setText(tr("The LinWin authorization helper is missing."));
        return;
    }
    const QString operation = uninstall ? QStringLiteral("uninstall")
        : needsDependencies ? QStringLiteral("install-xfce-deps") : QStringLiteral("install-session");
    pendingAction_ = operation;
    result_->setText(uninstall ? tr("Requesting package removal…")
        : needsDependencies ? tr("Installing the XFCE session components…") : tr("Registering the LinWin session…"));
    button(QWizard::CustomButton1)->setEnabled(false);
    button(QWizard::BackButton)->setEnabled(false);
    button(QWizard::NextButton)->setEnabled(false);
    button(QWizard::CancelButton)->setEnabled(false);
    helperOutput_.clear();
    helperProcess_->start(QStandardPaths::findExecutable(QStringLiteral("pkexec")), {helper, operation});
}

void SetupWizard::finishAction(int exitCode, QProcess::ExitStatus status)
{
    button(QWizard::BackButton)->setEnabled(true);
    button(QWizard::NextButton)->setEnabled(true);
    button(QWizard::CancelButton)->setEnabled(true);
    if (status == QProcess::NormalExit && exitCode == 0) {
        if (pendingAction_ == QStringLiteral("install-xfce-deps")) {
            result_->setText(tr("XFCE session components are installed. The system check has been refreshed; choose Install LinWin session… to continue."));
        } else if (uninstallMode_) {
            result_->setText(tr("LinWin has been removed. Log out and choose your previous desktop session at the login screen."));
            QTimer::singleShot(1000, this, []() { QCoreApplication::quit(); });
        } else {
            result_->setText(tr("LinWin is ready. Log out, choose LinWin in the session selector, and sign in. Your previous desktop remains available there."));
        }
    } else {
        helperOutput_ += QString::fromLocal8Bit(helperProcess_->readAllStandardError());
        const QString error = helperOutput_.trimmed();
        result_->setText(error.isEmpty()
            ? tr("The operation did not complete (exit code %1). No package changes were confirmed. Review the error and retry.").arg(exitCode)
            : error);
        button(QWizard::CustomButton1)->setEnabled(true);
    }
    updateReview();
}
