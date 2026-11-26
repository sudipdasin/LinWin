// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace LinWinVisual {
inline constexpr char vertexShader[] = R"GLSL(
attribute vec2 position;
varying vec2 uv;
void main() {
    uv = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
)GLSL";
inline constexpr char fragmentShader[] = R"GLSL(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 uv;
uniform float aspect;
uniform float glow;
void main() {
    vec2 p = vec2(uv.x, 1.0 - uv.y);
    vec2 q = (p - vec2(0.72, 0.43)) * vec2(aspect, 1.0);
    float halo = exp(-3.6 * dot(q, q));
    vec3 c = mix(vec3(0.026, 0.081, 0.18), vec3(0.055, 0.25, 0.46), clamp(p.x * 0.72 + (1.0-p.y)*0.28, 0.0, 1.0));
    c += vec3(0.025, 0.12, 0.22) * halo * glow;
    // Original angled light ribbons; no proprietary wallpaper or logo.
    float d = abs(q.y + 0.36*q.x - 0.12);
    float ribbon = exp(-32.0*d*d) * smoothstep(-0.65, 0.38, q.x);
    float edge = exp(-2600.0*pow(abs(d-0.095), 2.0));
    float second = exp(-70.0*pow(q.y - 0.55*q.x + 0.19, 2.0)) * smoothstep(-0.40, 0.45, q.x);
    c += glow * (vec3(0.027, 0.12, 0.23)*ribbon + vec3(0.04, 0.17, 0.26)*edge*0.45 + vec3(0.016, 0.08, 0.15)*second);
    float vignette = 1.0 - 0.23 * dot(p-0.5, p-0.5);
    gl_FragColor = vec4(clamp(c*vignette, 0.0, 1.0), 1.0);
}
)GLSL";
}
