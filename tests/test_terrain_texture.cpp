// SPDX-License-Identifier: GPL-3.0-or-later
#include "terrain_texture.h"
#include <iostream>
#include <set>
int main(){
    using LinWinTerrain::render;
    constexpr int w=640,h=360;
    const auto a=render(w,h),b=render(w,h);
    if(a.size()!=static_cast<std::size_t>(w*h)||a!=b||a.front()!=0||(a.back()>>24)!=255)return 1;
    if(!render(0,h).empty()||!render(2561,h).empty()||!render(w,1441).empty())return 2;
    std::set<std::uint32_t> terrain,lake;
    for(int y=static_cast<int>(h*.55);y<h;++y)for(int x=0;x<w;++x){
        const auto p=a[static_cast<std::size_t>(y*w+x)];
        if(p==0)continue;
        (y<h*LinWinTerrain::horizon?terrain:lake).insert(p);
    }
    if(terrain.size()<200||lake.size()<200)return 3;
    std::cout<<"PASS: deterministic bounded terrain, transparent sky, opaque lake and textured color variation\n";
}
