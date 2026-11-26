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
ctest --test-dir /tmp/linwin-build --output-on-failure
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
cpack --config /tmp/linwin-build/CPackConfig.cmake -G DEB -B /tmp/linwin-package -D CPACK_DEBIAN_PACKAGE_RELEASE=8+render.1 -D CPACK_PACKAGE_FILE_NAME=linwin-1.0.0-8+render.1-ubuntu-24.04-amd64
pkg=/tmp/linwin-package/linwin-1.0.0-8+render.1-ubuntu-24.04-amd64.deb
test "$(dpkg-deb -f "$pkg" Package)" = linwin
test "$(dpkg-deb -f "$pkg" Architecture)" = amd64
test "$(dpkg-deb -f "$pkg" Version)" = '1.0.0-8+render.1'
dpkg-deb -x "$pkg" /tmp/linwin-extracted
for f in embedded_worker.py linwin-update release_policy.py update_guard.py; do
    test -f "/tmp/linwin-extracted/usr/libexec/linwin/$f"
done
test -x /tmp/linwin-extracted/usr/bin/linwin
python3 -B /tmp/linwin-extracted/usr/libexec/linwin/embedded_worker.py --help
/tmp/linwin-extracted/usr/bin/linwin --version
cp "$pkg" /out/
cd /out
sha256sum ./*.deb > SHA256SUMS
printf '%s\n' 'Native Ubuntu 24.04 amd64 candidate. Build, CTest, updater tests and Xvfb startup checks passed. Not a Mint VM or physical GPU certification.' > validation.txt
