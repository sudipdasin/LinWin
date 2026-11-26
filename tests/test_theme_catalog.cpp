// SPDX-License-Identifier: GPL-3.0-or-later
#include "theme_catalog.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace LinWinTheme;
    assert(catalog.size() == 7);
    assert(byId("invalid").id == "neon");
    assert(byId("neon").widgets && byId("neon").effects);
    const auto &minimal = byId("minimal");
    assert(!minimal.effects && !minimal.widgets && minimal.light);
    assert(minimal.sky == minimal.horizon);
    for (const auto &a : catalog) {
        assert(byId(a.id).id == a.id);
        assert(a.sky.size() == 7 && a.horizon.size() == 7 && a.panel.size() == 7 && a.text.size() == 7 && a.accent.size() == 7);
        for (const auto &b : catalog) if (&a != &b) assert(a.id != b.id);
    }
    std::cout << "PASS: seven themes, unique IDs, default fallback, minimal performance policy, palette lengths\n";
}
