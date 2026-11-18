# Roadmap

LinWin's goal is a lightweight, all-in-one Linux shell that makes Windows switchers feel at home and supports users with no prior Linux or terminal knowledge. The current in-window panel and Start popup are interface previews, not a login-session shell or desktop-panel replacement.

## Architecture and install foundation

- [x] Choose a shared C++20/Qt 6 shell core that keeps the host Linux kernel and drivers
- [ ] Define a small shell process and optional, independently switchable components
- [ ] Add a comprehensive GUI installer that detects distro, kernel, package manager, session, hardware, and dependencies; installs matching packages; configures components; and provides rollback/uninstall
- [ ] Set measurable startup, idle CPU, memory, and interaction-latency budgets
- [ ] Publish verified distro, kernel, desktop, session, package, and component compatibility

## Shell experience

- [x] Add a responsive in-window bottom panel, Home/Files/Tasks/Clipboard navigation, and a searchable Start popup
- [ ] Replace the prototype window with a login-session shell and recovery/fallback path
- [x] Searchable Start-style app launcher using the host application registry, with persistent pinned apps and native launch behavior
- [ ] Integrate a host-session taskbar, Alt+Tab switching, live window tracking/previews, direct Bluetooth and accessibility controls, task manager, and notifications
- [x] Add a searchable Settings Hub that detects and opens installed native desktop settings tools
- [x] Add an X11 window-list adapter with taskbar activation and in-app Alt+Tab cycling when `wmctrl` is installed
- [ ] Add keyboard shortcuts and drag-and-drop through supported desktop/compositor interfaces
- [ ] Enable supported components by default in the standard install profile and let users switch each one off independently

## System integration

- [ ] Add hardware and service controls through documented Linux interfaces and system services
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
- [ ] Add process grouping, live-window awareness, richer resource history, and privileged system-process controls through desktop authorization

## Release readiness

- [x] Automated C++ builds on Ubuntu, Debian, Fedora, and Arch Linux containers
- [ ] Validate packages and shell startup/uninstall on supported desktops and session types
- [ ] Add automated tests for shared core behaviour and installer rollback
- [ ] Publish signed release artifacts, checksums, upgrade, rollback, and uninstall guidance
- [ ] Establish contributor and security response processes
