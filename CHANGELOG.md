# Changelog

All notable LinWin changes will be recorded here. This project is in early development and has not made a stable release.

## Unreleased

- Add a GUI-confirmed, allowlisted package-manager step for missing XFCE session components on Debian/Ubuntu, Fedora/RHEL, and Arch-family distributions.
- Show the exact base packages and detected package manager before authorization; unsupported distributions remain directed to their graphical software manager.
- Add distro-detection and command-plan regression coverage.

## 1.0.0-beta.3 - Accessible text scaling

- Add persistent LinWin text-size controls to Settings, adjustable from 85% to 150% with a one-click reset.
- Add standard zoom-in, zoom-out, and reset shortcuts; the preference applies throughout the shell and remains scoped to LinWin.
- Add UI regression coverage for scaling bounds, persistence, and reset behaviour.
- Version native packages as release 3 and verify DEB, RPM, and Arch package metadata in CI.

## 1.0.0-beta.2 - Responsive shell preview

- Make the taskbar adapt to narrow windows, moving secondary actions into a More menu while keeping core navigation available.
- Add a UI regression test for wide and narrow layouts, overflow navigation, and panel bounds.
- Restore automated GitHub Actions VM and four-distro checks on pull requests and main pushes; keep private pinned-image CI manual until verified.
- Version native packages as release 2 so beta.2 can upgrade beta.1 cleanly; validate Debian, RPM, and Arch package version metadata in CI.
- Update the public release status and workflow artifact labels for beta.2.

## 1.0.0-beta.1 - Major public testing preview

- Add persistent, default-on Start menu, File Manager, Task Manager, and Clipboard feature toggles in Settings; turning off a component immediately removes its taskbar entry and blocks its shortcut route.
- Pause clipboard capture and LinWin-only middle-click behavior whenever the Clipboard component is turned off, even if history capture was previously opted in.
- Add familiar Super, Super+E, and Ctrl+Shift+Esc shortcuts for Start, Files, and Task Manager.
- Add settings UI coverage for component defaults and preference persistence.
- Separate the GPL LinWin Free product from future paid work using a private companion repository and documented process-separation/licensing rules.
- Start the 1.0.0 public beta line as an installable, feedback-driven preview; this is not a stable release.
- Run the complete build, UI/session tests, package creation, and install/remove checks on pull requests and pushes to main.
- Gate private Pro source mirroring on a successful CI run for the exact public main commit.

- Add a GUI setup/uninstall wizard, a narrowly scoped polkit helper, and a selectable X11 session adapter built around XFCE's window manager and desktop.
- Add a full-screen shell launch mode, an in-session Sign out action, recovery guidance, and package-manager uninstall for APT, DNF, and pacman systems.
- Restrict the first shell-session setup to systems with the required XFCE tools; preserve the existing desktop session and leave unsupported Wayland/other-WM environments untouched.
- Add offscreen setup-review coverage and a session-wrapper smoke test; extend package install checks to cover the helper and X11 session files.
- Bump the project to 0.4.0 for the session integration preview.

- Grouped hardware- and service-dependent Network, Disk, Bluetooth, Startup apps, Services, and GPU Task Manager panels under a separate Experimental tab.
- Prepare installable DEB, RPM, and Arch x86_64 beta packages; test package install/removal in the matching distro containers and attach checksummed CI artifacts.
- Publish a desktop-menu entry and accurate app-preview package metadata; retain the host login session and existing desktop.

- Clarified the pre-beta status and documented the release blockers and safe evaluation limits.
- Run distro build and UI tests on pull requests, stage-install the binary, produce checksummed seven-day preview archives, and include the GPL license and README in install trees.

- Added a searchable Settings Hub for display, network, Bluetooth, sound, printers, power, accessibility, appearance, accounts, updates/software, and date/time; it detects installed native settings tools, opens them without shell commands or elevation, and clearly marks unavailable categories.
- Added a Settings taskbar page and Ctrl+, shortcut, plus a UI test for category search and empty-state behaviour.

- Expanded the clipboard center with pinned text clips, Super+V and Ctrl+Alt+V quick-open shortcuts while LinWin is active, and an opt-in middle-click paste preference limited to LinWin text fields.

- Added an X11 open-window taskbar strip with click-to-activate and an in-app Alt+Tab cycle when optional `wmctrl` is available; Wayland sessions keep their compositor's native switcher.

- Added direct BlueZ Bluetooth scanning and device actions (pair, connect, disconnect, trust, forget), with pairing prompts, bounded scan duration, and a desktop-settings fallback.
- Added a GPU tab that reads DRM/sysfs device identity, utilization, and video-memory counters when the active driver exposes them.
- Added Parent PID to the process table and exported process report so parent/child relationships are visible and reusable in diagnostics.
- Added per-process disk read/write rates from `/proc/<pid>/io`, with unavailable values identified and rates included in JSON exports.
- Added a lightweight task manager preview with live `/proc` process data, CPU and resident-memory usage, search, sorting, two-second refresh, confirmed same-user End task, and separately confirmed Force stop.
- Added a privacy-reviewed local JSON process report with atomic file saving; command-line details are disclosed before export.
- Added a read-only process details dialog with parent PID/name, executable, working directory, and command line; open it from Details or by double-clicking a row.
- Added Tasks to the shell panel and documented the preview's process-control limits.
- Added an initial file manager page with folder navigation, open, new folder, extension-aware rename selection, cut/copy/paste, symbolic-link shortcuts, system Trash, keyboard shortcuts, multi-select, context menus, and drag-and-drop.
- Added Files to the in-window shell panel and updated project status, usage guidance, and roadmap.
- Replaced the tab strip with a responsive shell workspace, persistent in-window bottom panel, Home/Clipboard navigation, live date/clock, and a floating searchable Start menu.
- Added keyboard-friendly Start search focus on opening and a clear Escape dismissal path when the main window has focus.
- Added a searchable Start-style application launcher that discovers registered apps, launches them through GIO, and stores pinned favorites.
- Defined the core audience as Windows switchers and Linux beginners who should be able to install and use LinWin without terminal or Bash knowledge.
- Specified a unified, familiar shell experience with automatic distro, kernel, hardware, and package detection in the GUI installer.
- Clarified the modular component defaults, performance direction, system-access boundary, recovery path, and graphical setup flow.
- Corrected prototype UI and package descriptions so they do not imply the complete shell is already implemented.

## Unreleased

- Make the taskbar responsive: compact windows move Clipboard, Settings, and Sign out into a More menu and hide nonessential window/system detail while preserving keyboard shortcuts.
- Add a Qt UI regression test for wide and compact taskbar layouts and overflow navigation.
- Correct the Start button tooltip to match the implemented Super and Ctrl+Esc shortcuts.

## 0.2.0 - Clipboard history preview

- Added an opt-in clipboard tab for recent text, with search, copy, remove, and clear actions.
- Stored up to 50 clips in local Qt application settings; capture is disabled by default and documented as unencrypted.
- Replaced the non-interactive sidebar labels with compact Overview and Clipboard tabs for narrow-window navigation.
- Added clear project documentation about capture lifetime, storage, and current limitations.

## 0.1.0 - Initial development scaffold

- Added a C++20 and Qt 6 desktop application.
- Added detection and display of distribution, desktop session, session type, and kernel.
- Added a responsive introductory dashboard and explicit development-status messaging.
- Added contributor, security, license, and roadmap documents.
- Added a container-based build workflow for Ubuntu, Debian, Fedora, and Arch Linux.
