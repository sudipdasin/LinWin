// SPDX-License-Identifier: GPL-3.0-or-later
#include "system_metrics.h"
#include "baseline.h"
#include <cstdlib>
#include <new>
#include <fstream>
#include <iostream>
#include <iterator>
static bool counting=false;
static unsigned long long allocations=0,allocatedBytes=0;
void* operator new(std::size_t n){if(counting){++allocations;allocatedBytes+=n;}if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete[](void*p)noexcept{std::free(p);}
void operator delete(void*p,std::size_t)noexcept{std::free(p);}
void operator delete[](void*p,std::size_t)noexcept{std::free(p);}
int main(){std::ifstream stat("/proc/stat"),mem("/proc/meminfo");std::string cpu;std::getline(stat,cpu);std::string memory((std::istreambuf_iterator<char>(mem)),{});if(cpu.empty()||memory.empty())return 1;
constexpr int n=1000;std::uint64_t oldSum=0,newSum=0;
// Warm up locale and parser paths before counting.
if(!Baseline::cpu(cpu)||!Baseline::memory(memory)||!LinWinMetrics::cpu(cpu)||!LinWinMetrics::memory(memory))return 2;
counting=true;for(int i=0;i<n;++i)oldSum+=Baseline::cpu(cpu)->idle+Baseline::memory(memory).value();counting=false;
auto oldAlloc=allocations,oldBytes=allocatedBytes;allocations=allocatedBytes=0;
counting=true;for(int i=0;i<n;++i)newSum+=LinWinMetrics::cpu(cpu)->idle+LinWinMetrics::memory(memory).value();counting=false;
std::cout<<"Iterations: "<<n<<" paired CPU/memory parses\nBaseline C++ heap allocations: "<<oldAlloc<<"\nOptimized C++ heap allocations: "<<allocations<<"\nBaseline requested bytes: "<<oldBytes<<"\nOptimized requested bytes: "<<allocatedBytes<<"\nEquivalent accumulated output: "<<(oldSum==newSum?"PASS":"FAIL")<<"\n";
return oldSum==newSum&&oldAlloc>0&&allocations==0?0:3;}
