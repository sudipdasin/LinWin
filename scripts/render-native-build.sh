#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
export QT_QPA_PLATFORM=offscreen PYTHONDONTWRITEBYTECODE=1
export XDG_CONFIG_HOME=/tmp/linwin-build-config XDG_DATA_HOME=/tmp/linwin-build-data XDG_CACHE_HOME=/tmp/linwin-build-cache
mkdir -p /out "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$XDG_CACHE_HOME"
test "$(dpkg --print-architecture)" = amd64
python3 -m unittest discover -s updater -v
cmake -S . -B /tmp/linwin-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="-O2 -UNDEBUG" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBEXECDIR=libexec -DBUILD_TESTING=ON
cmake --build /tmp/linwin-build --parallel 1
LINWIN_VISUAL_EVIDENCE_DIR=/out/visuals ctest --test-dir /tmp/linwin-build --output-on-failure
for theme in neon aero aurora familiar graphite cobalt minimal; do
    test -s "/out/visuals/$theme-start.png"
    test -s "/out/visuals/$theme-desktop.png"
done
/tmp/linwin-build/linwin --version
for mode in overlay shell; do
    code=0
    if [ "$mode" = shell ]; then
        timeout 5s xvfb-run -a env QT_QPA_PLATFORM=xcb XDG_SESSION_TYPE=x11 /tmp/linwin-build/linwin --shell || code=$?
    else
        timeout 5s xvfb-run -a env QT_QPA_PLATFORM=xcb XDG_SESSION_TYPE=x11 /tmp/linwin-build/linwin || code=$?
    fi
    test "$code" -eq 124
done
test -s /out/visuals/native-file-explorer.png
test -s /out/visuals/native-task-manager.png
test -s /out/visuals/native-task-view.png
test -s /out/visuals/native-quick-controls.png
cpack --config /tmp/linwin-build/CPackConfig.cmake -G DEB -B /tmp/linwin-package -D CPACK_DEBIAN_PACKAGE_RELEASE=8+render.10 -D CPACK_PACKAGE_FILE_NAME=linwin-1.0.0-8+render.10-ubuntu-24.04-amd64
pkg=/tmp/linwin-package/linwin-1.0.0-8+render.10-ubuntu-24.04-amd64.deb
test "$(dpkg-deb -f "$pkg" Package)" = linwin
test "$(dpkg-deb -f "$pkg" Architecture)" = amd64
test "$(dpkg-deb -f "$pkg" Version)" = '1.0.0-8+render.10'
dpkg-deb -x "$pkg" /tmp/linwin-extracted
for f in embedded_worker.py linwin-update release_policy.py update_guard.py; do
    test -f "/tmp/linwin-extracted/usr/libexec/linwin/$f"
done
test -x /tmp/linwin-extracted/usr/bin/linwin
python3 -B /tmp/linwin-extracted/usr/libexec/linwin/embedded_worker.py --help
/tmp/linwin-extracted/usr/bin/linwin --version
# Install the actual package in the disposable Ubuntu builder, not on the user's host.
apt-get update
apt-get install -y --no-install-recommends "$pkg"
test "$(dpkg-query -W -f='${Version}' linwin)" = '1.0.0-8+render.10'
test -f /usr/libexec/linwin/embedded_worker.py
/usr/bin/linwin --version
mkdir -p /out/benchmarks
for theme in neon minimal; do
    for sample in 1 2 3; do
        timeout 30s env QT_QPA_PLATFORM=offscreen /usr/bin/linwin --benchmark-native "$theme" > "/out/benchmarks/$theme-$sample.json"
    done
done
python3 scripts/aggregate-native-benchmarks.py /out/benchmarks
code=0
timeout 5s xvfb-run -a env QT_QPA_PLATFORM=xcb XDG_SESSION_TYPE=x11 /usr/bin/linwin || code=$?
test "$code" -eq 124
apt-get remove -y linwin
test ! -e /usr/bin/linwin
test ! -e /usr/libexec/linwin/embedded_worker.py
cp "$pkg" /out/
cp docs/render-gallery.html /out/index.html
python3 scripts/verify-visual-gallery.py /out
ln -s "$(basename "$pkg")" /out/linwin-latest.deb
cd /out
sha256sum "$(basename "$pkg")" > SHA256SUMS
printf '%s\n' 'Native Ubuntu 24.04 amd64 candidate. Build, CTest, updater tests, Xvfb startup, actual container APT install/remove, nineteen native Qt captures with effects enabled, native snap geometry/controller checks, recent-document, landscape and workspace restore, multiresolution icon and metric accessibility, Task View, native taskbar controls, persistent workspace, Quick Controls, Command Palette, per-tool geometry recall and PNG gallery integrity, icon-cache and six native benchmark process checks passed. Not a Mint VM or physical GPU certification.' > validation.txt
