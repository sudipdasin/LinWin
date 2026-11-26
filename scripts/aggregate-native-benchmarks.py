#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Aggregate real offscreen native measurements; reject structural regressions."""
import json
import statistics
import sys
from pathlib import Path

root = Path(sys.argv[1])
result = {"environment": "Ubuntu 24.04 amd64, Qt offscreen CPU fallback; not physical XFCE", "samples": 3, "themes": {}}
for theme in ("neon", "minimal"):
    rows = [json.loads((root / f"{theme}-{i}.json").read_text()) for i in range(1, 4)]
    for row in rows:
        assert row["theme"] == theme and row["platform"] == "offscreen"
        assert row["icon_requests"] == 800 and row["icon_uncached_pixmap_rasterizations"] == 3200
        assert row["icon_warm_rasterizations"] == 8 and row["icon_additional_cached_rasterizations"] == 0
        assert row["wallpaper_rebuilds_during_21_captures"] == 0
        assert row["resident_kib_after_settle"] > 0 and row["icon_cached_ms"] > 0
        assert 0 <= row["wallpaper_cache_bytes"] <= 2560*1440*4
        assert row["desktop_widget_polling_active"] == (theme == "neon")
        assert (row["wallpaper_cache_bytes"] == 0) == (theme == "minimal")
    numeric = ("construct_show_ms", "resident_kib_after_settle", "peak_rss_kib_including_benchmark", "wallpaper_cache_bytes",
               "capture_median_ms", "capture_p95_ms", "icon_uncached_ms", "icon_cached_ms")
    summary = {key: statistics.median(row[key] for row in rows) for key in numeric}
    summary["icon_lookup_speedup"] = summary["icon_uncached_ms"] / summary["icon_cached_ms"]
    result["themes"][theme] = {"median": summary, "raw_samples": rows}
(root.parent / "performance.json").write_text(json.dumps(result, indent=2) + "\n")
for theme, entry in result["themes"].items():
    m = entry["median"]
    print(f"BENCH {theme}: startup {m['construct_show_ms']:.2f} ms; RSS {m['resident_kib_after_settle']} KiB; wallpaper {m['wallpaper_cache_bytes']} bytes; capture {m['capture_median_ms']:.3f} ms; icons {m['icon_uncached_ms']:.2f} -> {m['icon_cached_ms']:.3f} ms ({m['icon_lookup_speedup']:.1f}x)")
print("PASS: all six native benchmark processes and resource invariants")
