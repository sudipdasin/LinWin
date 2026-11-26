// SPDX-License-Identifier: GPL-3.0-or-later
#include "system_metrics.h"
#include "baseline.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <cstdlib>
int main() {
    std::ifstream stat("/proc/stat"), mem("/proc/meminfo");
    std::string cpu; std::getline(stat,cpu);
    const std::string memory((std::istreambuf_iterator<char>(mem)),{});
    if(cpu.empty() || memory.empty()) return 1;
    const auto a=Baseline::cpu(cpu); const auto b=LinWinMetrics::cpu(cpu);
    if(!a||!b||a->total!=b->total||a->idle!=b->idle||Baseline::memory(memory)!=LinWinMetrics::memory(memory))return 2;
    using namespace LinWinMetrics;
    if(LinWinMetrics::cpu("cpu 18446744073709551615 1 0 0 0 0 0 0")||LinWinMetrics::cpu("cpu -1 0 0 0 0 0 0 0")||LinWinMetrics::cpu("cpu 1x 0 0 0 0 0 0 0"))return 3;
    if(LinWinMetrics::memory("MemTotal: 1000 kB\nMemAvailable: -1 kB\n")||LinWinMetrics::memory("MemTotal: 0 kB\nMemAvailable: 0 kB\n")||LinWinMetrics::memory("MemTotal: 1 kB\nMemAvailable: 2 kB\n"))return 4;
    if(LinWinMetrics::memory("MemTotal:\t1000 kB\nMemAvailable: 250 kB\n")!=75)return 5;
    for(unsigned i=1;i<=10000;++i) {
        std::string c="cpu "+std::to_string(i)+" 3 4 500 6 7 8 9 100 200";
        const auto old=Baseline::cpu(c);const auto now=LinWinMetrics::cpu(c);
        if(!old||!now||old->total!=now->total||old->idle!=now->idle)return 6;
        const std::string m="MemTotal: 10000 kB\nMemAvailable: "+std::to_string(i)+" kB\n";
        if(Baseline::memory(m)!=LinWinMetrics::memory(m))return 7;
    }
    std::cout<<"PASS: 20000 synthetic equivalence cases, live procfs sample, malformed and overflow rejection\n";
    constexpr int iterations=10000; std::uint64_t checksum=0;
    auto measure=[&](auto fn) {
        const auto start=std::chrono::steady_clock::now();
        for(int i=0;i<iterations;++i) checksum+=fn();
        return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    };
    std::vector<double> oldTimes,newTimes;
    auto old=[&](){return Baseline::cpu(cpu)->idle+Baseline::memory(memory).value();};
    auto now=[&](){return LinWinMetrics::cpu(cpu)->idle+LinWinMetrics::memory(memory).value();};
    for(int r=0;r<7;++r){if(r%2==0){oldTimes.push_back(measure(old));newTimes.push_back(measure(now));}else{newTimes.push_back(measure(now));oldTimes.push_back(measure(old));}}
    std::sort(oldTimes.begin(),oldTimes.end());std::sort(newTimes.begin(),newTimes.end());
    std::cout<<"Iterations per trial: "<<iterations<<"; trials: 7 alternating\n"<<"Baseline median ms: "<<oldTimes[3]<<"\nOptimized median ms: "<<newTimes[3]<<"\nSpeedup: "<<oldTimes[3]/newTimes[3]<<"x\nChecksum: "<<checksum<<"\n";
}
