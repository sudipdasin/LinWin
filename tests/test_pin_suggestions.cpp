// SPDX-License-Identifier: GPL-3.0-or-later
#include "pin_suggestions.h"
#include <iostream>
using namespace LinWinPins;
int main(){
    const std::vector<App> apps{{"firefox.desktop",{"WebBrowser"}},{"chromium.desktop",{"WebBrowser"}},{"unknown-mail.desktop",{"Email"}},{"thunar.desktop",{"FileManager"}},{"z-editor.desktop",{"TextEditor"}},{"a-editor.desktop",{"TextEditor"}},{"unrelated.desktop",{"Game"}},{"",{"Calendar"}}};
    if(suggestions(apps,{})!=std::vector<std::string>{"firefox.desktop","unknown-mail.desktop","a-editor.desktop","thunar.desktop"})return 1;
    if(suggestions(apps,{"chromium.desktop","uninstalled.desktop"})!=std::vector<std::string>{"unknown-mail.desktop","a-editor.desktop","thunar.desktop"})return 2;
    if(suggestions(apps,{},2)!=std::vector<std::string>{"firefox.desktop","unknown-mail.desktop"})return 3;
    if(!suggestions(apps,{"firefox.desktop","firefox.desktop"},1).empty()||!suggestions({},{}).empty()||!suggestions(apps,{},0).empty())return 4;
    auto reversed=apps;std::reverse(reversed.begin(),reversed.end());if(suggestions(reversed,{})!=suggestions(apps,{}))return 5;
    const std::vector<App> cross{{"multi.desktop",{"WebBrowser","Email"}}};if(suggestions(cross,{}).size()!=1)return 6;
    std::cout<<"PASS: preferred installed IDs, category fallback, deterministic order, existing pin preservation, missing IDs, limits and duplicate prevention\n";
}
