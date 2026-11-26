#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Run inside a checked-out LinWin source tree on Mint 22 / Ubuntu 24.04 amd64.
set -eu
cd "$(dirname "$0")/.."
[ "$(dpkg --print-architecture)" = amd64 ] || { echo "amd64 required" >&2; exit 1; }
. /etc/os-release
case "${ID:-}:${VERSION_ID:-}:${UBUNTU_CODENAME:-}" in
  linuxmint:22*:noble|ubuntu:24.04:*) ;;
  *) echo "This build helper targets Mint 22.x / Ubuntu 24.04 only." >&2; exit 1 ;;
esac
for tool in cmake ctest cpack g++ pkg-config dpkg-deb xvfb-run timeout git; do
  command -v "$tool" >/dev/null || { echo "Missing tool: $tool" >&2; exit 1; }
done
pkg-config --exists Qt6Widgets gio-unix-2.0 || {
  echo "Install qt6-base-dev and libglib2.0-dev first." >&2; exit 1;
}
rev=$(git rev-parse --short=12 HEAD)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
out="$PWD/dist"
mkdir -p "$out"
cmake -S . -B "$work/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build "$work/build" --parallel 2
ctest --test-dir "$work/build" --output-on-failure
QT_QPA_PLATFORM=offscreen "$work/build/linwin" --version
status=0
xvfb-run -a timeout 5s env XDG_SESSION_TYPE=x11 "$work/build/linwin" || status=$?
[ "$status" -eq 124 ] || { echo "Overlay startup failed ($status)." >&2; exit 1; }
# Candidate version sorts above beta.8 and below beta.9. No release tag is claimed.
name="linwin-local-$rev-ubuntu-24.04"
cpack --config "$work/build/CPackConfig.cmake" -G DEB -B "$work/packages" \
  -D "CPACK_DEBIAN_PACKAGE_RELEASE=8+local.$rev" -D "CPACK_PACKAGE_FILE_NAME=$name"
pkg="$work/packages/$name.deb"
[ "$(dpkg-deb -f "$pkg" Package)" = linwin ]
[ "$(dpkg-deb -f "$pkg" Architecture)" = amd64 ]
[ "$(dpkg-deb -f "$pkg" Version)" = "1.0.0-8+local.$rev" ]
cp "$pkg" "$out/"
(cd "$out" && sha256sum "$name.deb" > "$name.deb.sha256")
printf 'Candidate package: %s/%s.deb\n' "$out" "$name"
printf 'No privileged install performed. Real XFCE desktop and rollback testing still required.\n'
