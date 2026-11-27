// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <xcb/xcb.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
namespace LinWinDock {
struct Reservation {
    std::array<uint32_t,12> values{};
    bool valid=false;
};
// Single full-width X11 screen only. Never reserve an already occupied edge.
inline Reservation reserve(int width,int height,int bar,bool top,int occupied) {
    Reservation result;
    if(width<1||height<1||bar<1||bar>=height||occupied!=0)return result;
    result.values[top?2:3]=static_cast<uint32_t>(bar);
    result.values[top?8:10]=0;
    result.values[top?9:11]=static_cast<uint32_t>(width-1);
    result.valid=true;return result;
}
class Connection {
public:
    Connection(){int index=0;c_=xcb_connect(nullptr,&index);if(!c_||xcb_connection_has_error(c_))return;
        auto iter=xcb_setup_roots_iterator(xcb_get_setup(c_));while(index--&&iter.rem)xcb_screen_next(&iter);if(iter.rem)root_=iter.data->root;}
    ~Connection(){if(c_)xcb_disconnect(c_);}
    Connection(const Connection&)=delete;Connection& operator=(const Connection&)=delete;
    bool valid() const{return c_&&root_!=XCB_NONE&&!xcb_connection_has_error(c_);}
    xcb_window_t root() const{return root_;}
    xcb_atom_t atom(const char *name) const {
        if(!valid())return XCB_NONE;
        auto *reply=xcb_intern_atom_reply(c_,xcb_intern_atom(c_,0,static_cast<uint16_t>(std::strlen(name)),name),nullptr);
        const auto result=reply?reply->atom:static_cast<xcb_atom_t>(XCB_NONE);std::free(reply);return result;
    }
    bool read(xcb_window_t window,const char *name,xcb_atom_t type,uint32_t *out,uint32_t count) const {
        if(!valid()||!window||!out||!count)return false;
        const auto property=atom(name);if(!property)return false;
        auto *reply=xcb_get_property_reply(c_,xcb_get_property(c_,0,window,property,type,0,count),nullptr);
        const bool ok=reply&&reply->type==type&&reply->format==32&&reply->value_len==count&&reply->bytes_after==0;
        if(ok)std::memcpy(out,xcb_get_property_value(reply),count*sizeof(uint32_t));
        std::free(reply);return ok;
    }
    bool wmPresent() const {
        uint32_t wm=0,self=0;
        return read(root_,"_NET_SUPPORTING_WM_CHECK",XCB_ATOM_WINDOW,&wm,1)&&wm&&read(wm,"_NET_SUPPORTING_WM_CHECK",XCB_ATOM_WINDOW,&self,1)&&self==wm;
    }
    bool write(xcb_window_t window,const char *name,xcb_atom_t type,const uint32_t *data,uint32_t count) const {
        const auto property=atom(name);if(!property||!window||!data||!count)return false;
        auto *error=xcb_request_check(c_,xcb_change_property_checked(c_,XCB_PROP_MODE_REPLACE,window,property,type,32,count,data));
        const bool ok=!error;std::free(error);return ok;
    }
    void clear(xcb_window_t window) const {
        if(!valid()||!window)return;
        for(const auto *name:{"_NET_WM_STRUT","_NET_WM_STRUT_PARTIAL","_NET_WM_WINDOW_TYPE"}){
            const auto property=atom(name);if(property){auto *error=xcb_request_check(c_,xcb_delete_property_checked(c_,window,property));std::free(error);}
        }
        xcb_flush(c_);
    }
    bool apply(xcb_window_t window,const Reservation &reservation) const {
        if(!valid()||!window||!reservation.valid)return false;
        const uint32_t type=atom("_NET_WM_WINDOW_TYPE_DOCK");
        if(!type||!write(window,"_NET_WM_WINDOW_TYPE",XCB_ATOM_ATOM,&type,1)||
           !write(window,"_NET_WM_STRUT",XCB_ATOM_CARDINAL,reservation.values.data(),4)||
           !write(window,"_NET_WM_STRUT_PARTIAL",XCB_ATOM_CARDINAL,reservation.values.data(),12)){
            clear(window);return false;
        }
        xcb_flush(c_);return true;
    }
    xcb_connection_t *native() const{return c_;}
private:
    xcb_connection_t *c_=nullptr;xcb_window_t root_=XCB_NONE;
};
}
