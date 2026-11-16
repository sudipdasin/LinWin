// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QString>

struct SystemProfile {
    QString distribution;
    QString distributionId;
    QString desktopEnvironment;
    QString sessionType;
    QString kernel;

    [[nodiscard]] static SystemProfile detect();
};
