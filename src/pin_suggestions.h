// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>
namespace LinWinPins {
struct App { std::string id; std::vector<std::string> categories; };
struct Role { std::string_view category; std::array<std::string_view,5> preferred; };
inline constexpr std::array<Role,13> roles{{
    {"WebBrowser",{"firefox.desktop","org.mozilla.firefox.desktop","brave-browser.desktop","chromium.desktop","google-chrome.desktop"}},
    {"Email",{"thunderbird.desktop","org.mozilla.Thunderbird.desktop","org.gnome.Evolution.desktop","evolution.desktop","claws-mail.desktop"}},
    {"Calendar",{"org.gnome.Calendar.desktop","org.kde.korganizer.desktop","orage.desktop","",""}},
    {"WordProcessor",{"libreoffice-writer.desktop","org.libreoffice.LibreOffice.writer.desktop","abiword.desktop","",""}},
    {"Photography",{"org.gnome.Photos.desktop","org.gnome.eog.desktop","ristretto.desktop","org.xfce.ristretto.desktop","gthumb.desktop"}},
    {"Player",{"vlc.desktop","org.videolan.VLC.desktop","mpv.desktop","org.gnome.Totem.desktop","rhythmbox.desktop"}},
    {"RasterGraphics",{"gimp.desktop","org.gimp.GIMP.desktop","org.kde.krita.desktop","krita.desktop","pinta.desktop"}},
    {"TextEditor",{"org.xfce.mousepad.desktop","mousepad.desktop","org.gnome.TextEditor.desktop","xed.desktop","org.kde.kate.desktop"}},
    {"TerminalEmulator",{"xfce4-terminal.desktop","org.xfce.Terminal.desktop","org.gnome.Terminal.desktop","konsole.desktop","xterm.desktop"}},
    {"PackageManager",{"mintinstall.desktop","synaptic.desktop","org.gnome.Software.desktop","org.kde.discover.desktop",""}},
    {"Settings",{"xfce-settings-manager.desktop","xfce4-settings-manager.desktop","org.gnome.Settings.desktop","cinnamon-settings.desktop","systemsettings.desktop"}},
    {"Calculator",{"galculator.desktop","org.gnome.Calculator.desktop","org.kde.kcalc.desktop","mate-calc.desktop","xfce4-calculator.desktop"}},
    {"FileManager",{"thunar.desktop","org.xfce.Thunar.desktop","nemo.desktop","org.gnome.Nautilus.desktop","org.kde.dolphin.desktop"}}
}};
// Return only new, installed IDs. Preserve existing pins, including unavailable
// IDs, in caller storage. Never duplicate or silently replace an existing pin.
inline std::vector<std::string> suggestions(std::vector<App> apps,const std::vector<std::string>& existing,std::size_t limit=12) {
    std::sort(apps.begin(),apps.end(),[](const App&a,const App&b){return a.id<b.id;});
    auto contains=[](const auto&v,const auto&x){return std::find(v.begin(),v.end(),x)!=v.end();};
    auto installed=[&](std::string_view id){return std::any_of(apps.begin(),apps.end(),[&](const App&a){return !a.id.empty()&&a.id==id;});};
    std::vector<std::string> chosen;
    for(const auto&id:existing)if(installed(id)&&!contains(chosen,id))chosen.push_back(id);
    std::vector<std::string> additions;
    for(const auto&role:roles){
        if(chosen.size()>=limit)break;
        auto matches=[&](const App&a){return contains(role.preferred,std::string_view(a.id))||contains(a.categories,role.category);};
        auto eligible=[](const App&a){return a.id!="linwin-setup.desktop"&&a.id!="linwin-uninstall.desktop"&&a.id!="linwin-update.desktop"&&a.id!="linwin-update-autostart.desktop";};
        if(std::any_of(apps.begin(),apps.end(),[&](const App&a){return contains(chosen,a.id)&&matches(a);}))continue;
        std::string next;
        for(auto id:role.preferred)if(!id.empty()&&installed(id)&&!contains(chosen,id)){next=id;break;}
        if(next.empty())for(const auto&a:apps)if(!a.id.empty()&&eligible(a)&&!contains(chosen,a.id)&&matches(a)){next=a.id;break;}
        if(!next.empty()){chosen.push_back(next);additions.push_back(next);}
    }
    return additions;
}
}
