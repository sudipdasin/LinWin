#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# No network, CI, or privileged installation.
set -eu
cd "$(dirname "$0")/../updater"
for tool in python3 dpkg-deb dpkg; do
    command -v "$tool" >/dev/null || { echo "Missing tool: $tool" >&2; exit 1; }
done
export PYTHONDONTWRITEBYTECODE=1
python3 -m unittest discover -v
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
sh packaging/build.sh "$work/updater.deb"
[ "$(dpkg-deb -f "$work/updater.deb" Package)" = linwin-update-center ]
[ "$(dpkg-deb -f "$work/updater.deb" Architecture)" = all ]
expected=$(sed -n 's/^Version: //p' packaging/control)
[ "$(dpkg-deb -f "$work/updater.deb" Version)" = "$expected" ]
dpkg-deb -x "$work/updater.deb" "$work/root"
python3 "$work/root/usr/lib/linwin-update-center/linwin-update" --help
printf '%s\n' 'PASS: updater tests, package metadata and extracted executable. Native desktop and privileged installation not tested.'
