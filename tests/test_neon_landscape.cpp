// SPDX-License-Identifier: GPL-3.0-or-later
#include "neon_landscape.h"
#include <QImage>
#include <iostream>
int main() {
    auto render=[](){
        QImage img(640,360,QImage::Format_RGB32); img.fill(QColor("#03152e"));
        QPainter painter(&img); LinWinVisual::drawNeonLandscape(painter,img.rect()); painter.end();
        return img;
    };
    const auto a=render(),b=render();
    if(a!=b || a.pixelColor(10,350)==QColor("#03152e") || a.pixelColor(10,10)!=QColor("#03152e")) return 1;
    if(a.pixelColor(300,260)==a.pixelColor(300,350)) return 2;
    std::cout<<"PASS: deterministic native landscape, retained sky, lake depth and nonuniform output\n";
}
