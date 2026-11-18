# Roadmap

LinWin's goal is a lightweight, all-in-one Linux shell that makes Windows switchers feel at home and supports users with no prior Linux or terminal knowledge. The first experimental login-session adapter targets X11 with XFCE tools; it is not yet a broadly supported desktop-panel replacement.

## Product editions

- [x] Keep LinWin Free as a complete, useful, GPL-3.0-or-later application in the public repository
- [x] Establish the private `LinWin-Pro` repository for proprietary source, licensed resources, and paid-product work
- [ ] Automate a read-only hourly snapshot sync of public `main` into private `public-core/`
- [ ] Publish a versioned, open process-boundary interface before developing integrations that depend on it
- [ ] Keep LinWin Free buildable and usable without Pro; distribute any Pro client as a separate optional package
- [ ] Review copyright, dependency, packaging, and GPL/Qt obligations before any commercial binary distribution

## Architecture and install foundation

- [x] Choose a shared C++20/Qt 6 shell core that keeps the host Linux kernel and drivers
- [ ] Define a small shell process and optional, independently switchable components
- [x] Add an initial GUI setup/uninstall wizard that detects host details, verifies XFCE/X11 prerequisites, preserves the previous session, and uses a narrow polkit helper
- [ ] Extend the wizard to install missing dependencies, choose per-component defaults, and recover transactionally from failed setup
- [ ] Set measurable startup, idle CPU, memory, and interaction-latency budgets
- [ ] Publish verified distro, kernel, desktop, session, package, and component compatibility

## Shell experience

- [x] Add a responsive in-window bottom panel, Home/Files/Tasks/Clipboard navigation, and a searchable Start popup
- [x] Add a full-screen X11 login-session preview using xfwm4/xfdesktop, a LinWin taskbar, and a user-confirmed sign-out path to the display manager
- [ ] Validate login, relogin, failed startup, multi-monitor, sleep/wake, accessibility, and rollback on real XFCE desktops
- [x] Searchable Start-style app launcher using the host application registry, with persistent pinned apps and native launch behavior
- [ ] Integrate a host-session taskbar, Alt+Tab switching, live window tracking/previews, direct Bluetooth and accessibility controls, task manager, and notifications
- [x] Add a searchable Settings Hub that detects and opens installed native desktop settings tools
- [x] Add an X11 window-list adapter with taskbar activation and in-app Alt+Tab cycling when `wmctrl` is installed
- [ ] Add keyboard shortcuts and drag-and-drop through supported desktop/compositor interfaces
- [x] Enable core shell components by default and let users switch Start, Files, Task Manager, and Clipboard off independently in Settings

## System integration

- [ ] Add broader hardware and service controls through documented Linux interfaces and system services
- [ ] Add Bluetooth pairing through BlueZ and network/audio/power integration through host services
- [ ] Add accessible zoom, text scaling, and high-contrast controls
- [ ] Add safe configuration links and narrow, authenticated privileged actions
- [ ] Keep hardware drivers, device firmware, and kernel maintenance owned by the host distribution

## User-facing components

- [x] Add opt-in local text clipboard history with search, copy, remove, clear, up to 50 recent and 50 pinned items, and a LinWin-only middle-click paste preference
- [ ] Add pinned and image/file clips, background capture adapters, and finer privacy controls
- [x] Add a basic file manager preview with folder navigation, rename, copy/cut/paste, shortcut links, system Trash, keyboard actions, and drag-and-drop
- [ ] Add file-operation progress, conflict choices, undo where safe, file search, and richer drag-and-drop feedback
- [x] Add a lightweight task manager preview with `/proc` CPU/memory sampling, search, sorting, safe same-user close, and confirmed force-stop
- [x] Group hardware/service-dependent Network, Disk, Bluetooth, Startup apps, Services, and GPU monitoring under an Experimental tab
- [ ] Add process grouping, live-window awareness, richer resource history, and privileged system-process controls through desktop authorization

## Release readiness

- [x] Automated C++ builds, UI tests, and native package install/removal on Ubuntu, Debian, Fedora, and Arch Linux x86_64 containers
- [x] Publish short-lived checksummed native package artifacts from pull-request CI
- [ ] Validate desktop startup, shell-session behavior, upgrades, and uninstall on real X11/XFCE systems; add adapters for other supported desktops and Wayland compositors
- [ ] Add automated tests for shared core behaviour and installer rollback
- [ ] Publish signed broader beta artifacts with upgrade, rollback, shell-session recovery, and uninstall guidance after real desktop validation
- [ ] Establish contributor and security response processes
