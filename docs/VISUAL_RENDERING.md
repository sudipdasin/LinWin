# LinWin visual rendering foundation

Original blue light-ribbon wallpaper, real GLSL vertex and fragment programs, bounded offscreen OpenGL rendering with QPainter fallback. Right-click desktop → Visual effects controls lighting and preferred OpenGL; choices persist per user. The wallpaper is cached, resampled on resize and never continuously animated; high-DPI pixel targets are capped at 2560×1440. No proprietary Microsoft assets. The taskbar has a subtle gradient, highlight and a light Start surface.

OpenGL is an optional Qt module; the CPU fallback remains available without it and when context/shader creation fails. This is wallpaper shading, not compositor-level acrylic or blur of external windows. Native display management, fractional scaling across screens, HDR, color management, composition effects, animation and GPU hardware testing remain later milestones.

The exact shader was compiled, linked and rendered independently in Mesa llvmpipe; opaque, nonuniform output and changed lighting pixels verified with no GL error. Qt integration and DEB compilation are pending because Qt 6 development libraries are missing. No GitHub Actions or CircleCI was invoked.
