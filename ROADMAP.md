# Roadmap

This roadmap describes intent; it is not a promise that every item is already available.

## Foundation

- [x] C++20 and Qt 6 application scaffold
- [x] Detect distribution, desktop session, and kernel for support diagnostics
- [x] Public contribution, security, and licensing documentation
- [x] Automated C++ build on Ubuntu, Debian, Fedora, and Arch containers
- [ ] Validate and publish installable packages on Debian/Ubuntu/Mint, Fedora, and Arch
- [ ] Publish verified compatibility matrix by distro, desktop, and session type

## Core usability tools

- [x] Opt-in local text clipboard history with search, copy, remove, clear, and a 50-item limit
- [ ] Pinned clips, image/file formats, finer privacy controls, and configurable middle-click selection paste
- [ ] Optional background capture with explicit session/desktop permission support
- [ ] Cross-distro file-operation helper with clear copy/move/link feedback and undo where supported
- [ ] Searchable settings hub that opens native settings panels when appropriate
- [ ] Bluetooth pairing flow built on the system Bluetooth service
- [ ] Accessible zoom, text scaling, and high-contrast controls

## Desktop integrations

- [ ] Cinnamon integration (Linux Mint)
- [ ] Xfce integration (Linux Mint and other Xfce distributions)
- [ ] KDE Plasma integration
- [ ] MATE integration
- [ ] Launcher, taskbar, Alt+Tab, shortcuts, and drag-and-drop adapters, each with explicit support status

## Release readiness

- [ ] Automated build checks across supported distro images
- [ ] Automated tests for shared core behaviour
- [ ] Signed release artifacts and checksums
- [ ] Upgrade, rollback, and uninstall documentation
- [ ] Security contact and vulnerability response process
