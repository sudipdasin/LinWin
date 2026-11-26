// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <string_view>
namespace LinWinTheme {
struct Theme { std::string_view id, name, sky, horizon, panel, text, accent; bool effects, widgets, light; };
inline constexpr std::array<Theme, 7> catalog{{
    {"neon", "Neon Fluent", "#03152e", "#075a9d", "#09233e", "#edf7ff", "#29dfff", true, true, false},
    {"aero", "Aero Slate", "#071b42", "#1865ac", "#152b4d", "#edf5ff", "#65baff", true, false, false},
    {"aurora", "Aurora Glass", "#14234d", "#1b8c9e", "#183c59", "#f0faff", "#76f4e8", true, false, false},
    {"familiar", "Classic Familiar", "#0752a4", "#259cea", "#e9f1fb", "#172940", "#247aca", false, false, true},
    {"graphite", "Graphite Pro", "#10151e", "#364c65", "#17222f", "#edf2f8", "#86c5ff", true, false, false},
    {"cobalt", "Cobalt Light", "#0755bc", "#8cc8f2", "#f1f5fb", "#162b49", "#246ac0", true, false, true},
    {"minimal", "Classic Minimal Fast", "#365a83", "#365a83", "#eeeeee", "#202020", "#245f9a", false, false, true}
}};
constexpr const Theme &byId(std::string_view id) {
    for (const auto &t : catalog) if (t.id == id) return t;
    return catalog.front();
}
}
