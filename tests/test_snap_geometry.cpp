// SPDX-License-Identifier: GPL-3.0-or-later
#include "snap_geometry.h"
#include <iostream>
int main() {
    using namespace LinWinSnap;
    const Rect a{-1920,40,1919,1001};
    const Insets zero{0,0,0,0}, frame{5,30,5,5};
    if(target(a,Zone::Left,zero,1,1)!=Rect{-1920,40,959,1001}) return 1;
    if(target(a,Zone::Right,zero,1,1)!=Rect{-961,40,960,1001}) return 2;
    if(target(a,Zone::TopLeft,zero,1,1)!=Rect{-1920,40,959,500}) return 3;
    if(target(a,Zone::TopRight,zero,1,1)!=Rect{-961,40,960,500}) return 4;
    if(target(a,Zone::BottomLeft,zero,1,1)!=Rect{-1920,540,959,501}) return 5;
    if(target(a,Zone::BottomRight,zero,1,1)!=Rect{-961,540,960,501}) return 6;
    if(target(a,Zone::Fill,frame,1,1)!=Rect{-1915,70,1909,966}) return 7;
    if(target(a,Zone::Left,frame,950,1)) return 8;
    if(target(a,Zone::Fill,{-1,0,0,0},1,1)) return 9;
    if(target({0,0,0,1},Zone::Fill,zero,1,1)) return 10;
    if(target(a,Zone::Fill,{2000,0,0,0},1,1)) return 11;
    if(target(a,Zone::Fill,zero,-1,1)) return 12;
    std::cout<<"PASS: 12 snap geometry checks, odd screen sizes, negative coordinates, insets and minimum size rejection\n";
}
