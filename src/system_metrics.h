// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <optional>
#include <string_view>
#include <charconv>
#include <cstdint>
#include <limits>
namespace LinWinMetrics {
struct Cpu { std::uint64_t total, idle; };
inline bool space(char c) { return c==' ' || c=='\t' || c=='\r' || c=='\n'; }
inline std::string_view token(std::string_view &s) {
    while(!s.empty() && space(s.front())) s.remove_prefix(1);
    std::size_t n=0; while(n<s.size() && !space(s[n])) ++n;
    const auto result=s.substr(0,n); s.remove_prefix(n); return result;
}
inline bool number(std::string_view s,std::uint64_t &out) {
    if(s.empty()) return false;
    const auto parsed=std::from_chars(s.data(),s.data()+s.size(),out);
    return parsed.ec==std::errc{} && parsed.ptr==s.data()+s.size();
}
inline std::optional<Cpu> cpu(std::string_view line) {
    if(token(line)!="cpu") return {};
    std::uint64_t total=0,idle=0;
    for(int i=0;i<8;++i) {
        std::uint64_t n=0;
        if(!number(token(line),n) || n>std::numeric_limits<std::uint64_t>::max()-total) return {};
        total+=n;
        if(i==3 || i==4) idle+=n;
    }
    return Cpu{total,idle};
}
inline std::optional<int> usage(Cpu prev,Cpu next) {
    if(next.total<=prev.total || next.idle<prev.idle || next.idle-prev.idle>next.total-prev.total) return {};
    return static_cast<int>(100.0*(1.0-static_cast<double>(next.idle-prev.idle)/static_cast<double>(next.total-prev.total))+0.5);
}
inline std::optional<int> memory(std::string_view text) {
    std::uint64_t total=0,available=0; bool haveTotal=false,haveAvailable=false;
    while(!text.empty()) {
        const auto end=text.find('\n'); auto row=text.substr(0,end);
        if(end==std::string_view::npos) text={}; else text.remove_prefix(end+1);
        const auto key=token(row);
        if(key!="MemTotal:" && key!="MemAvailable:") continue;
        std::uint64_t n=0;
        if(!number(token(row),n) || token(row)!="kB") continue;
        if(key=="MemTotal:") {total=n;haveTotal=true;}
        else {available=n;haveAvailable=true;}
    }
    if(!haveTotal || !total || !haveAvailable || available>total) return {};
    return static_cast<int>(100.0*(1.0-static_cast<double>(available)/static_cast<double>(total))+0.5);
}
}
