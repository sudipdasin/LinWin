// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace LinWinTerrain {
struct RGB { double r,g,b; };
inline RGB mix(RGB a,RGB b,double t){return {a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t};}
inline RGB add(RGB a,RGB b,double t){return {a.r+b.r*t,a.g+b.g*t,a.b+b.b*t};}
inline double clamp(double x){return std::clamp(x,0.0,1.0);}
inline double smooth(double x){x=clamp(x);return x*x*(3-2*x);}
inline double hash(int x,int y,int seed){std::uint32_t n=static_cast<std::uint32_t>(x)*374761393u+static_cast<std::uint32_t>(y)*668265263u+static_cast<std::uint32_t>(seed)*2246822519u;n=(n^(n>>13))*1274126177u;return static_cast<double>(n^(n>>16))/4294967295.0;}
inline double noise(double x,double y,int seed){const int a=static_cast<int>(std::floor(x)),b=static_cast<int>(std::floor(y));const double u=smooth(x-a),v=smooth(y-b);const double p=hash(a,b,seed)*(1-u)+hash(a+1,b,seed)*u,q=hash(a,b+1,seed)*(1-u)+hash(a+1,b+1,seed)*u;return p*(1-v)+q*v;}
inline double fbm(double x,double y,int seed){double v=0,w=.53;for(int i=0;i<5;++i){v+=w*noise(x,y,seed+i*19);x=x*2.04+13.7;y=y*2.07-7.1;w*=.48;}return v;}
inline constexpr double horizon=.792;
inline constexpr std::array<double,17> farPeaks{.74,.70,.67,.69,.64,.68,.73,.69,.65,.61,.65,.57,.55,.62,.66,.69,.68};
inline constexpr std::array<double,17> mainPeaks{.77,.73,.71,.74,.71,.68,.74,.76,.70,.66,.61,.64,.55,.60,.66,.69,.72};
inline constexpr std::array<double,17> forePeaks{.71,.73,.74,.76,.77,.78,.79,.791,.79,.79,.78,.77,.75,.73,.69,.68,.69};
inline double edge(const std::array<double,17>& a,double x,int seed){const double f=clamp(x)*16;const int i=std::min(15,static_cast<int>(f));const double t=f-i;return a[i]*(1-t)+a[i+1]*t+(fbm(x*65,7,seed)-.48)*.023;}
inline double relief(double x,double y,int seed){const double warp=(fbm(x*10,y*12,seed+17)-.48)*.08;const double u=x+(y-.58)*.75+warp;const double r=1-std::abs(2*fbm(u*75,y*18,seed)-1);return .55*r+.45*fbm(x*240,y*160,seed+71);}
inline RGB mountain(double x,double y,double top,int seed,int layer){const double depth=clamp((y-top)/std::max(.01,horizon-top));const double h=relief(x,y,seed);const double slope=(relief(x+.0018,y,seed)-relief(x-.0018,y,seed))*3.5;const double exposure=clamp(.50+slope+(h-.5)*.7);const double snow=smooth((exposure-.42)*2.7)*smooth((h-.39)*3.1)*std::pow(1-depth,.65);const RGB dark=layer==2?RGB{2,10,22}:RGB{4,17,38};const RGB rock=layer==0?RGB{18,54,91}:RGB{12,42,75};RGB c=mix(dark,rock,exposure*.72+.08);if(layer!=2)c=add(c,{11,96,166},snow*(layer==1?1.22:.55));c=mix(c,dark,depth*.46);return c;}
inline RGB land(double x,double y){const double a=edge(farPeaks,x,12),b=edge(mainPeaks,x,34),c=edge(forePeaks,x,58);RGB color{0,0,0};if(y>=a)color=mountain(x,y,a,12,0);if(y>=b)color=mountain(x,y,b,34,1);if(y>=c)color=mountain(x,y,c,58,2);return color;}
inline std::vector<std::uint32_t> render(int width,int height){if(width<1||height<1||width>2560||height>1440)return {};std::vector<std::uint32_t> data(static_cast<std::size_t>(width)*static_cast<std::size_t>(height),0);const auto pack=[](RGB c){auto b=[](double n){return static_cast<std::uint32_t>(std::clamp(n,0.0,255.0)+.5);};return 0xff000000u|(b(c.r)<<16)|(b(c.g)<<8)|b(c.b);};
// Render only the terrain and lake; alpha zero retains the separately rendered sky/ribbon.
for(int row=static_cast<int>(height*.51);row<height;++row){const double y=static_cast<double>(row)/(height-1>0?height-1:1);for(int col=0;col<width;++col){const double x=static_cast<double>(col)/(width-1>0?width-1:1);RGB color;if(y<horizon){color=land(x,y);if(color.r==0&&color.g==0&&color.b==0)continue;}else{const double d=(y-horizon)/(1-horizon);const double shift=(noise(x*11,y*530,91)-.5)*(.006+.035*d);const RGB reflected=land(clamp(x+shift),horizon-(y-horizon)*.9);color=mix({2,11,27},reflected,.40*(1-d*.48));const double wave=fbm(x*8,y*510,81);const double streak=std::exp(-std::pow((x-.60)/(.10+.06*d),2))*smooth((wave-.23)*2.1);const double shafts=.35+.65*std::pow(noise(x*31,y*4,58),2);color=add(color,{3,133,218},streak*shafts*(1-d*.52));const double shore=std::exp(-std::pow((y-horizon-.002)/.0028,2));color=add(color,{12,150,205},shore*std::exp(-std::pow((x-.61)/.21,2)));}data[static_cast<std::size_t>(row)*width+col]=pack(color);}}return data;}
}
