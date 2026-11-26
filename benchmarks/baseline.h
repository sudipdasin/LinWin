#pragma once
#include <optional>
#include <sstream>
#include <string>
#include <cstdint>
namespace Baseline {
struct Cpu { std::uint64_t total, idle; };
inline std::optional<Cpu> cpu(const std::string &line) {
    std::istringstream in(line); std::string label;
    std::uint64_t user=0,nice=0,system=0,idle=0,wait=0,irq=0,soft=0,steal=0;
    if (!(in>>label>>user>>nice>>system>>idle>>wait>>irq>>soft>>steal) || label!="cpu") return {};
    return Cpu{user+nice+system+idle+wait+irq+soft+steal,idle+wait};
}
inline std::optional<int> memory(const std::string &text) {
    std::istringstream in(text); std::string line; std::uint64_t total=0,available=0; bool found=false;
    while(std::getline(in,line)) { std::istringstream row(line); std::string key,unit; std::uint64_t value=0;
        if(!(row>>key>>value>>unit) || unit!="kB") continue;
        if(key=="MemTotal:") total=value;
        if(key=="MemAvailable:") { available=value; found=true; }
    }
    if(!total || !found || available>total) return {};
    return static_cast<int>(100.0*(1.0-static_cast<double>(available)/static_cast<double>(total))+0.5);
}
}
