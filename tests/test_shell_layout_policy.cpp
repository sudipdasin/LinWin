// SPDX-License-Identifier: GPL-3.0-or-later
#include "shell_layout_policy.h"
#include <iostream>
int main(){
    using namespace LinWinLayout;
    unsigned long cases=0;
    for(int w=320;w<=3840;w+=8)for(int h=240;h<=2160;h+=8)
        for(bool neon:{false,true})for(bool full:{false,true})for(bool top:{false,true}){
            const auto m=start(w,h,neon,full,top);
            if(m.x<0||m.y<0||m.width<1||m.height<1||m.x+m.width>w||m.y+m.height>h)return 1;
            if(full&&(m.x||m.y||m.width!=w||m.height!=h))return 2;
            if(!full){const int inset=w>650?6:0;if(m.y!=(top?inset:h-m.height-inset))return 6;}
            ++cases;
        }
    if(start(1920,1022,true,false).width!=666||start(1672,883,true,false).width!=580)return 3;
    if(taskbarHeight(true,false)!=58||taskbarHeight(true,true)!=44||taskbarHeight(false,false)!=44)return 4;
    if(!compactWindows(true,false)||compactWindows(true,true)||compactWindows(false,false))return 5;
    std::cout<<"PASS: "<<cases<<" menu geometry combinations; top/bottom anchoring, full-screen, reference widths and taskbar policy\n";
}
