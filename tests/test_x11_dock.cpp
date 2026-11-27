// SPDX-License-Identifier: GPL-3.0-or-later
#include "x11_dock.h"
#include <iostream>
int main(int argc,char **){
    using namespace LinWinDock;
    unsigned long count=0;
    for(int width=320;width<=3840;width+=16)for(int height=240;height<=2160;height+=16)
        for(int bar:{32,44,58,72})for(bool top:{false,true}){
            const auto r=reserve(width,height,bar,top,0);
            if(!r.valid||r.values[top?2:3]!=static_cast<uint32_t>(bar)||r.values[top?3:2]!=0||r.values[top?9:11]!=static_cast<uint32_t>(width-1))return 1;
            if(reserve(width,height,bar,top,1).valid)return 2;
            ++count;
        }
    if(reserve(0,1080,58,false,0).valid||reserve(1920,20,58,true,0).valid||reserve(1920,1080,0,false,0).valid)return 3;
    std::cout<<"PASS: "<<count<<" edge reservation geometries and occupied-edge rejection\n";
    if(argc==1)return 0;
    Connection c;if(!c.valid())return 4;
    auto *conn=c.native();const auto window=xcb_generate_id(conn);
    auto *error=xcb_request_check(conn,xcb_create_window_checked(conn,XCB_COPY_FROM_PARENT,window,c.root(),0,0,320,44,0,XCB_WINDOW_CLASS_INPUT_OUTPUT,XCB_COPY_FROM_PARENT,0,nullptr));
    if(error){std::free(error);return 5;}
    const auto desired=reserve(1920,1080,44,false,0);
    if(!c.apply(window,desired))return 6;
    std::array<uint32_t,12> actual{};uint32_t type=0;
    if(!c.read(window,"_NET_WM_STRUT_PARTIAL",XCB_ATOM_CARDINAL,actual.data(),12)||actual!=desired.values||!c.read(window,"_NET_WM_WINDOW_TYPE",XCB_ATOM_ATOM,&type,1)||type!=c.atom("_NET_WM_WINDOW_TYPE_DOCK"))return 7;
    c.clear(window);
    if(c.read(window,"_NET_WM_STRUT_PARTIAL",XCB_ATOM_CARDINAL,actual.data(),12))return 8;
    xcb_destroy_window(conn,window);xcb_flush(conn);
    std::cout<<"PASS: real X11 dock type and partial strut property write, read and cleanup\n";
}
