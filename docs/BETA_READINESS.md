# LinWin 1.0 public beta readiness

Last reviewed: 2026-11-19

## Release state

LinWin 1.0.0-beta.2 is an installable public testing preview, not a stable desktop replacement. The app can run as a normal desktop application. Its optional login-session adapter supports XFCE on X11 and keeps the user's original session available as a recovery path. Wayland and non-XFCE login sessions are not supported by the adapter.

The graphical setup flow checks for required XFCE tools and registers or removes the selectable session. It does not install missing desktop dependencies. Package upgrade, rollback, real login-manager integration, and recovery across hardware and display-manager combinations remain incomplete.

## Automated test scope

Every pull request and push to `main` runs a direct Ubuntu runner-VM build, C++ UI/session tests, CLI startup check, and a short Xvfb shell-launch smoke test. A four-distribution package matrix builds and tests packages on Ubuntu 24.04, Debian 13, Fedora, and Arch Linux x86_64, including package install/remove checks. These checks verify the code and package transactions; they do not verify a physical desktop session.

The private LinWin Pro mirror is intended to import a public `main` revision only after CI reports success for that exact commit. The private sync workflow runs on an hourly schedule and can also be manually dispatched. This automation depends on GitHub Actions being available for the private repository.

## Known coverage limits

| Area | Current behavior | Remaining limits |
| --- | --- | --- |
| Start, taskbar, and shortcuts | Familiar Start-style launcher, pinned apps, responsive taskbar overflow, and common keyboard shortcuts | Window-management behavior varies by desktop and session |
| Files | Browse, rename, copy/cut/paste, trash, shortcuts, and drag-and-drop | Cross-toolkit drag-and-drop and unusual mount types need broader testing |
| Clipboard | Optional local text history and LinWin-scoped paste behavior | Saved text is not encrypted; clipboard integration varies by session |
| Task Manager | Live process/resource views and same-user task actions | Network, disk, Bluetooth, startup, services, and GPU panels are experimental and may be unavailable on some systems; this is not yet equivalent to Windows Task Manager |
| Window switching | X11 window listing and in-app Alt+Tab when `wmctrl` is installed; host compositor switcher otherwise | No compositor-independent window-control layer |
| Settings and devices | Opens available host tools and reads selected system interfaces | Depends on distro packages, permissions, services, hardware, and desktop session |
| Setup and uninstall | GUI setup/uninstall flow with a narrowly scoped authorization helper | Real polkit prompts, login-manager behavior, upgrade, rollback, and recovery need more VM and hardware coverage |

## Safe evaluation

Test in a virtual machine or secondary account. Keep your existing desktop session selectable. Do not rely on this preview as the only way to administer or recover a system. Report issues with the LinWin version, distribution and version, desktop and display session, relevant hardware, and reproduction steps. Remove names, local paths, hostnames, network identifiers, serials, and tokens from diagnostics before posting them.

## Work before stable 1.0

- Validate login, lock, logout, sleep/wake, multi-monitor, scaling, keyboard navigation, and accessibility on a named support matrix.
- Test package upgrade, failed-upgrade recovery, rollback, and GUI uninstall on clean VMs for each supported distribution.
- Add dependency installation and per-component selection to setup where appropriate.
- Improve and test service, startup, Bluetooth, network, disk, and GPU panels on representative systems.
- Confirm release artifacts are checksummed and that the source tree contains no unintended private data.
