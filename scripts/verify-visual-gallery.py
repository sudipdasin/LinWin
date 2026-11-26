#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Validate captured native PNGs and bind the public gallery to its evidence."""
import hashlib
import json
import struct
import sys
import zlib
from html.parser import HTMLParser
from pathlib import Path

THEMES = ("neon", "aero", "aurora", "familiar", "graphite", "cobalt", "minimal")
TOOLS = ("native-file-explorer", "native-task-manager", "native-task-view",
         "native-quick-controls", "native-command-palette")
EXPECTED = {f"visuals/{theme}-{mode}.png" for theme in THEMES for mode in ("desktop", "start")}
EXPECTED.update(f"visuals/{tool}.png" for tool in TOOLS)


class Gallery(HTMLParser):
    def __init__(self):
        super().__init__()
        self.images = []
        self.links = []

    def handle_starttag(self, tag, attrs):
        data = dict(attrs)
        if tag == "img":
            self.images.append(data.get("src", ""))
        if tag == "a" and data.get("href", "").startswith("visuals/"):
            self.links.append(data["href"])


def inspect_png(path):
    data = path.read_bytes()
    if not data.startswith(b"\x89PNG\r\n\x1a\n") or len(data) > 50 * 1024 * 1024:
        raise ValueError(f"Invalid PNG signature or size: {path}")
    pos = 8
    width = height = 0
    saw_data = saw_end = False
    while pos + 12 <= len(data):
        length = struct.unpack_from(">I", data, pos)[0]
        end = pos + 12 + length
        if end > len(data):
            raise ValueError(f"Truncated PNG chunk: {path}")
        kind = data[pos + 4:pos + 8]
        payload = data[pos + 8:end - 4]
        crc = struct.unpack_from(">I", data, end - 4)[0]
        if zlib.crc32(kind + payload) & 0xffffffff != crc:
            raise ValueError(f"Bad PNG chunk CRC: {path}")
        if pos == 8:
            if kind != b"IHDR" or length != 13:
                raise ValueError(f"Missing PNG header: {path}")
            width, height = struct.unpack_from(">II", payload)
            if not (100 <= width <= 8192 and 100 <= height <= 8192):
                raise ValueError(f"Unexpected native capture dimensions: {path}")
        if kind == b"IDAT":
            saw_data = True
        if kind == b"IEND":
            saw_end = length == 0 and end == len(data)
            break
        pos = end
    if not (saw_data and saw_end):
        raise ValueError(f"Incomplete PNG: {path}")
    return {"width": width, "height": height, "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}


def verify(root):
    parser = Gallery()
    parser.feed((root / "index.html").read_text(encoding="utf-8"))
    if set(parser.images) != EXPECTED or len(parser.images) != len(EXPECTED):
        raise ValueError("Gallery images do not match the required captures")
    if set(parser.links) != EXPECTED or len(parser.links) != len(EXPECTED):
        raise ValueError("Gallery image links do not match the required captures")
    actual = {str(p.relative_to(root)) for p in (root / "visuals").glob("*.png")}
    if actual != EXPECTED:
        raise ValueError("Captured PNG inventory does not match the gallery")
    records = [{"path": name, **inspect_png(root / name)} for name in sorted(EXPECTED)]
    (root / "visual-manifest.json").write_text(json.dumps({"captures": records}, indent=2) + "\n", encoding="utf-8")
    print(f"PASS: {len(records)} gallery captures, PNG structure, chunk CRC, dimensions and SHA256")


if __name__ == "__main__":
    verify(Path(sys.argv[1]))
