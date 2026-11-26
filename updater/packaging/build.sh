#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
root=$(mktemp -d)
trap 'rm -rf "$root"' EXIT HUP INT TERM
install -D -m 0644 packaging/control "$root/DEBIAN/control"
install -D -m 0755 linwin-update "$root/usr/lib/linwin-update-center/linwin-update"
install -D -m 0644 release_policy.py "$root/usr/lib/linwin-update-center/release_policy.py"
mkdir -p "$root/usr/bin"
ln -s ../lib/linwin-update-center/linwin-update "$root/usr/bin/linwin-update"
install -D -m 0644 packaging/linwin-update.desktop "$root/usr/share/applications/linwin-update.desktop"
install -D -m 0644 packaging/linwin-update-autostart.desktop "$root/etc/xdg/autostart/linwin-update.desktop"
install -D -m 0644 README.md "$root/usr/share/doc/linwin-update-center/README.md"
if [ -f /usr/share/common-licenses/GPL-3 ]; then
  install -D -m 0644 /usr/share/common-licenses/GPL-3 "$root/usr/share/doc/linwin-update-center/copyright"
fi
printf '/etc/xdg/autostart/linwin-update.desktop\n' > "$root/DEBIAN/conffiles"
chmod 644 "$root/DEBIAN/conffiles"
dpkg-deb --root-owner-group --build "$root" "${1:-linwin-update-center_1.0.1-1_all.deb}"
