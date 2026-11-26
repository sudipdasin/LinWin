// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
namespace LinWinLayout {
struct Menu { int x,y,width,height; };
inline Menu start(int width,int height,bool neon,bool fullscreen,bool top=false) {
    width=std::max(1,width);height=std::max(1,height);
    if(fullscreen)return {0,0,width,height};
    const int inset=width>650?6:0;
    const bool reference=neon&&width>=1450;
    const int targetWidth=reference?std::clamp(static_cast<int>(width*.347+.5),560,720):660;
    const int targetHeight=reference?std::clamp(static_cast<int>(height*.61+.5),540,660):635;
    const int w=std::max(1,std::min(targetWidth,width-2*inset));
    const int h=std::max(1,std::min(targetHeight,height-2*inset));
    return {inset,top?std::min(inset,height-h):std::max(0,height-h-inset),w,h};
}
inline int taskbarHeight(bool premium,bool small){return small?44:premium?58:44;}
inline bool compactWindows(bool premium,bool labels){return premium&&!labels;}
}
