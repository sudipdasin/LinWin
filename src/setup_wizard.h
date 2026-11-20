// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWizard>
#include <QProcess>
#include <QString>

class QLabel;

class SetupWizard final : public QWizard {
    Q_OBJECT
public:
    explicit SetupWizard(bool uninstallMode = false, QWidget *parent = nullptr);

private:
    void updateReview();
    void runAction(bool uninstall);
    void finishAction(int exitCode, QProcess::ExitStatus status);

    bool uninstallMode_ = false;
    QLabel *compatibility_ = nullptr;
    QLabel *review_ = nullptr;
    QLabel *result_ = nullptr;
    QProcess *helperProcess_ = nullptr;
    QString helperOutput_;
    QString pendingAction_;
};
