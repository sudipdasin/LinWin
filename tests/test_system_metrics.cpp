// SPDX-License-Identifier: GPL-3.0-or-later
#include "system_metrics.h"
#include <iostream>
int main() {
    using namespace LinWinMetrics;
    auto a=cpu("cpu 10 0 5 80 5 0 0 0 8 0"), b=cpu("cpu 20 0 15 155 10 0 0 0 10 0");
    if(!a || !b || usage(*a,*b)!=20 || usage(*b,*a) || usage(*a,*a) || cpu("cpu0 10 0 5 80 5 0 0 0") || cpu("cpu invalid")) return 1;
    if(memory("MemTotal: 1000 kB\nMemAvailable: 400 kB\n")!=60 || memory("MemTotal: 1000 kB\n") || memory("MemTotal: 0 kB\nMemAvailable: 0 kB\n") || memory("MemTotal: 100 kB\nMemAvailable: 200 kB\n")) return 2;
    std::cout<<"PASS: CPU deltas, reset handling, malformed inputs, memory accounting and unavailable data\n";
}
