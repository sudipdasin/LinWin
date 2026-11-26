# LinWin integration candidate — 2026-11-26

This is an isolated test snapshot, NOT a published release, validated shell, or merge into main. It combines non-overlapping source from the native desktop/clipboard/Settings/Start dismissal workstream, refreshed Start app registry, and Update Center 1.0.3 including the complete offline tests. Original development branches remain intact.

## Local gates (no GitHub Actions or CircleCI)

1. Run `sh scripts/test-updater-local.sh`: full 39-case suite, build companion DEB, inspect metadata, extract and run help. This gate passed locally in the development environment. It does not simulate actual privileged installation.
2. On Mint 22.3 XFCE amd64, run `sh scripts/build-xfce-deb.sh` for CMake, CTest, Xvfb startup and native DEB. Not run: Qt6Widgets and gio-unix-2.0 dev dependencies unavailable in current environment.
3. In a clean XFCE VM, install both candidates graphically, open overlay, verify Start search, Escape, outside click, desktop item selection and open, application registry refresh after install/remove, clipboard search and mouse behavior, and update Settings entry. Verify logout, return to Linux desktop, and removal. Test actual driver/device behavior separately; no universal Windows hardware compatibility claim.
4. Verify candidate version ordering, explicit update consent, dependency changes, signed-release trust provisioning, failure recovery, and installation through a real desktop polkit agent.
5. Before any main promotion, verify all gates, confirm both prohibited CI services cannot be triggered by branch updates or release publication, and preserve existing package recovery.

No new desktop binary was generated. Existing beta.8 remains the published native package; updater 1.0.3 is a separate companion. Full independent floating windows, taskbar parity, system-wide middle mouse behavior, plug-and-play and rollback remain pending.
