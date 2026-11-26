// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <optional>
namespace LinWinSnap {
struct Rect { int x,y,w,h; constexpr bool operator==(const Rect&) const = default; };
struct Insets { int left,top,right,bottom; };
enum class Zone { Left, Right, TopLeft, TopRight, BottomLeft, BottomRight, Fill };
constexpr std::optional<Rect> target(Rect a, Zone zone, Insets m, int minW, int minH) {
    if(a.w<=0 || a.h<=0 || m.left<0 || m.top<0 || m.right<0 || m.bottom<0 || minW<0 || minH<0) return {};
    const int w=a.w/2,h=a.h/2;
    Rect r=a;
    switch(zone) {
        case Zone::Left: r.w=w; break;
        case Zone::Right: r.x+=w; r.w-=w; break;
        case Zone::TopLeft: r.w=w; r.h=h; break;
        case Zone::TopRight: r.x+=w; r.w-=w; r.h=h; break;
        case Zone::BottomLeft: r.w=w; r.y+=h; r.h-=h; break;
        case Zone::BottomRight: r.x+=w; r.w-=w; r.y+=h; r.h-=h; break;
        case Zone::Fill: break;
    }
    if(m.left>=r.w || m.right>=r.w-m.left || m.top>=r.h || m.bottom>=r.h-m.top) return {};
    r.x+=m.left; r.y+=m.top; r.w-=m.left+m.right; r.h-=m.top+m.bottom;
    if(r.w<minW || r.h<minH) return {};
    return r;
}
}
