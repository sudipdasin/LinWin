# Roadmap

LinWin's goal is a lightweight, all-in-one Linux shell that makes Windows switchers feel at home and supports users with no prior Linux or terminal knowledge. The dashboard and clipboard page are prototypes, not the complete shell.

## Architecture and install foundation

- [x] Choose a shared C++20/Qt 6 shell core that keeps the host Linux kernel and drivers
- [ ] Define a small shell process and optional, independently switchable components
- [ ] Add a comprehensive GUI installer that detects distro, kernel, package manager, session, hardware, and dependencies; installs matching packages; configures components; and provides rollback/uninstall
- [ ] Set measurable startup, idle CPU, memory, and interaction-latency budgets
- [ ] Publish verified distro, kernel, desktop, session, package, and component compatibility

## Shell experience

- [ ] Replace the prototype window with a login-session shell and recovery/fallback path
- [ ] Build an integrated taskbar, Start-style application launcher, Alt+Tab switching, file workflows, clipboard, Bluetooth, accessibility, task manager, settings, and notifications
- [ ] Add keyboard shortcuts and drag-and-drop through supported desktop/compositor interfaces
- [ ] Enable supported components by default in the standard install profile and let users switch each one off independently

## System integration

- [ ] Add hardware and service controls through documented Linux interfaces and system services
- [ ] Add Bluetooth pairing through BlueZ and network/audio/power integration through host services
- [ ] Add accessible zoom, text scaling, and high-contrast controls
- [ ] Add safe configuration links and narrow, authenticated privileged actions
- [ ] Keep hardware drivers, device firmware, and kernel maintenance owned by the host distribution

## User-facing components

- [x] Prototype opt-in local text clipboard history with search, copy, remove, clear, and a 50-item limit
- [ ] Add pinned and image/file clips, background capture adapters, and finer privacy controls
- [ ] Add file manager workflows for rename, copy/paste, shortcuts, and clear drag-and-drop feedback
- [ ] Add a full task manager with safe app-close and advanced system-process views

## Release readiness

- [x] Automated C++ builds on Ubuntu, Debian, Fedora, and Arch Linux containers
- [ ] Validate packages and shell startup/uninstall on supported desktops and session types
- [ ] Add automated tests for shared core behaviour and installer rollback
- [ ] Publish signed release artifacts, checksums, upgrade, rollback, and uninstall guidance
- [ ] Establish contributor and security response processes
