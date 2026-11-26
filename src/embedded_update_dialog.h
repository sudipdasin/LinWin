// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDialog>
#include <QComboBox>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <QVBoxLayout>

#ifndef LINWIN_UPDATE_WORKER_PATH
#define LINWIN_UPDATE_WORKER_PATH "/usr/libexec/linwin/embedded_worker.py"
#endif

// All user-facing update controls belong to LinWin. The private worker speaks
// newline-delimited JSON over pipes and never creates a separate application.
class EmbeddedUpdateDialog final : public QDialog {
public:
    explicit EmbeddedUpdateDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setObjectName(QStringLiteral("linwinEmbeddedUpdateDialog"));
        setWindowTitle(tr("LinWin Updates"));
        resize(590, 450);
        auto *layout = new QVBoxLayout(this);
        auto *description = new QLabel(tr("Check official releases, verify the package and approve installation. Restart LinWin after a successful update."), this);
        description->setWordWrap(true);
        layout->addWidget(description);
        auto *row = new QHBoxLayout;
        row->addWidget(new QLabel(tr("Release channel"), this));
        channel_ = new QComboBox(this);
        channel_->setObjectName(QStringLiteral("linwinUpdateChannel"));
        channel_->addItem(tr("Preview (beta)"), QStringLiteral("preview"));
        channel_->addItem(tr("Stable"), QStringLiteral("stable"));
        QSettings settings;
        channel_->setCurrentIndex(settings.value(QStringLiteral("updates/channel"), QStringLiteral("preview")).toString() == QStringLiteral("stable") ? 1 : 0);
        row->addWidget(channel_, 1);
        check_ = new QPushButton(tr("Check for updates"), this);
        check_->setObjectName(QStringLiteral("linwinCheckForUpdates"));
        row->addWidget(check_);
        layout->addLayout(row);
        status_ = new QPlainTextEdit(this);
        status_->setObjectName(QStringLiteral("linwinUpdateStatus"));
        status_->setReadOnly(true);
        status_->setPlainText(tr("Ready to check for updates."));
        layout->addWidget(status_, 1);
        auto *actions = new QHBoxLayout;
        yes_ = new QPushButton(tr("Approve"), this);
        no_ = new QPushButton(tr("Decline"), this);
        close_ = new QPushButton(tr("Close"), this);
        yes_->setObjectName(QStringLiteral("linwinUpdateApprove"));
        no_->setObjectName(QStringLiteral("linwinUpdateDecline"));
        close_->setObjectName(QStringLiteral("linwinUpdateClose"));
        actions->addWidget(yes_);
        actions->addWidget(no_);
        actions->addStretch();
        actions->addWidget(close_);
        layout->addLayout(actions);
        yes_->setEnabled(false);
        no_->setEnabled(false);
        worker_ = new QProcess(this);
        connect(check_, &QPushButton::clicked, this, [this]() { begin(); });
        connect(yes_, &QPushButton::clicked, this, [this]() { answer(true); });
        connect(no_, &QPushButton::clicked, this, [this]() { answer(false); });
        connect(close_, &QPushButton::clicked, this, &QDialog::reject);
        connect(worker_, &QProcess::readyReadStandardOutput, this, [this]() { readEvents(); });
        connect(worker_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int code, QProcess::ExitStatus exitStatus) {
            readEvents();
            if (code != 0 || exitStatus != QProcess::NormalExit)
                status_->appendPlainText(tr("Update did not complete. No successful installation is claimed."));
            else if (!receivedEvent_)
                status_->appendPlainText(tr("Update check finished without a result."));
            setBusy(false);
        });
        connect(worker_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                status_->appendPlainText(tr("The bundled update worker could not start."));
                setBusy(false);
            }
        });
    }

protected:
    void reject() override
    {
        // Do not kill the worker while a package transaction may be running.
        if (worker_->state() == QProcess::NotRunning) QDialog::reject();
    }

private:
    void setBusy(bool busy)
    {
        check_->setEnabled(!busy);
        channel_->setEnabled(!busy);
        close_->setEnabled(!busy);
        yes_->setEnabled(false);
        no_->setEnabled(false);
        awaiting_ = false;
    }

    void begin()
    {
        if (worker_->state() != QProcess::NotRunning) return;
        const QString python = QStringLiteral("/usr/bin/python3");
        const QString workerPath = QStringLiteral(LINWIN_UPDATE_WORKER_PATH);
        if (!QFileInfo(python).isExecutable() || !QFileInfo(workerPath).isFile()) {
            status_->setPlainText(tr("The bundled update engine is missing. Reinstall LinWin using its official package."));
            return;
        }
        QSettings settings;
        const QString channel = channel_->currentData().toString();
        settings.setValue(QStringLiteral("updates/channel"), channel);
        receivedEvent_ = false;
        status_->setPlainText(tr("Checking official LinWin releases…"));
        setBusy(true);
        worker_->start(python, {QStringLiteral("-B"), workerPath, QStringLiteral("--channel"), channel});
    }

    void readEvents()
    {
        while (worker_->canReadLine()) {
            const auto doc = QJsonDocument::fromJson(worker_->readLine());
            if (!doc.isObject()) continue;
            const auto event = doc.object();
            const QString kind = event.value(QStringLiteral("kind")).toString();
            const QString message = event.value(QStringLiteral("message")).toString();
            if (kind != QStringLiteral("question") && kind != QStringLiteral("info") && kind != QStringLiteral("error")) continue;
            receivedEvent_ = true;
            status_->appendPlainText(message);
            if (kind == QStringLiteral("question")) {
                awaiting_ = true;
                yes_->setEnabled(true);
                no_->setEnabled(true);
                no_->setFocus(Qt::OtherFocusReason);
            }
        }
    }

    void answer(bool approved)
    {
        if (!awaiting_ || worker_->state() != QProcess::Running) return;
        awaiting_ = false;
        yes_->setEnabled(false);
        no_->setEnabled(false);
        status_->appendPlainText(approved ? tr("Approved. Working…") : tr("Declined. No further action authorized."));
        worker_->write(approved ? "yes\n" : "no\n");
    }

    QComboBox *channel_ = nullptr;
    QPushButton *check_ = nullptr;
    QPushButton *yes_ = nullptr;
    QPushButton *no_ = nullptr;
    QPushButton *close_ = nullptr;
    QPlainTextEdit *status_ = nullptr;
    QProcess *worker_ = nullptr;
    bool awaiting_ = false;
    bool receivedEvent_ = false;
};
