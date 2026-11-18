# LinWin architecture

LinWin is a lightweight, all-in-one desktop-shell replacement for Linux users who want familiar Windows-style workflows without needing to learn Linux, terminals, or Bash. It owns the main shell experience while continuing to use the host distribution's kernel, drivers, services, applications, and package manager.

## Product experience

Bring common desktop actions together in one consistent graphical shell: a taskbar, Start-style launcher, Alt+Tab switching, file operations, clipboard, Bluetooth, accessibility, task manager, and system settings. The current launcher prototype searches the host desktop registry and delegates launch behavior to GIO instead of parsing `Exec` fields itself. Favor familiar labels and visible actions. Normal setup and daily use must not require terminal commands or shell scripts. Keep advanced controls available without making beginners depend on them.

## Layers

1. **Shell core:** one C++20/Qt 6 session process for the panel, launcher, overview, notifications, shortcuts, and shell settings. Keep this process small and responsive.
2. **Optional components:** clipboard, file workflows, Bluetooth, device controls, task manager, and accessibility features. Ship them in the standard install profile enabled by default, but let users disable each independently. Load or activate them only when practical; do not create a permanent daemon for every feature.
3. **Desktop/session adapters:** connect to the active X11 or Wayland session and its supported window manager/compositor interfaces. Adapters declare their compatibility; the shared shell must not assume that one desktop's private APIs work everywhere. The first window-list adapter uses optional `wmctrl` on X11; Wayland continues to use its compositor-native switcher until a compositor adapter is available.
4. **Host-system interfaces:** detect the host distribution, release, CPU architecture, kernel, desktop session, package manager, hardware, and available services. Use existing kernel drivers and documented Linux userspace interfaces, system buses, portals, and services. Examples include udev for device discovery, BlueZ for Bluetooth, and the host's network, audio, power, and login services.
5. **Narrow privileged helper:** only add one for an operation that genuinely requires elevated privileges. Keep the UI and normal shell process unprivileged, authenticate through the host authorization mechanism, limit each helper action, and report the result.

## Free and Pro boundary

The public repository contains the entire LinWin Free shell under GPL-3.0-or-later. LinWin Pro is developed in the private `sudipdasin/LinWin-Pro` repository; a scheduled sync keeps an unmodified `public-core/` snapshot aligned with public `main`, while all private implementation and licensed assets live under `proprietary/`. Changes to the free core are made and reviewed in the public project first. Never commit private source, credentials, customer data, or restricted assets to this repository.

The planned Pro edition is a separate executable and package. It may add optional applications and integration points, but LinWin Free must continue to build, install, and work by itself. Prefer documented OS interfaces such as D-Bus, portals, and standard desktop entries, using a versioned protocol and explicit user consent for privileged operations. Do not load a private shared library into the GPL process, link private code into the shell executable, exchange complex shared-memory structures, or make the free shell depend on a license service. Process separation is an architectural aim, not a legal guarantee; the actual product boundary and distribution model need qualified license review before paid binaries ship.

The Pro team must not relicense community contributions without explicit permission from every relevant rightsholder. Public contributions remain available under the repository's GPL license. Keep copyright notices, third-party license records, and asset provenance with both products.

## Hardware and kernel access

“Direct access” means LinWin can expose and coordinate hardware and system controls from one shell using the host's supported interfaces, while the installer selects packages compatible with the detected distribution and supported kernel. Device drivers, firmware loading, kernel updates, and low-level device ownership remain with the Linux kernel and distribution. The shell must not bypass drivers with arbitrary device writes or run as root. Read-only kernel information can be shown where useful; changes that require privilege must go through a narrowly scoped, authenticated system operation.

## Component model

- Keep components separately identifiable, individually switchable, and reversible.
- The installer detects the host and selects matching packages and session configuration; users should not need to choose package names or type commands.
- The installer enables the supported standard component set by default and allows users to turn items off during setup.
- The settings hub allows components to be turned on or off after installation.
- An unavailable component is shown with the missing dependency or unsupported session; it must not silently start a duplicate service.
- Keep shared data formats and interfaces documented so contributors can add or improve components without tying the core to one distribution.

## Performance and reliability

Avoid duplicate shell processes, background polling, heavyweight bundled frameworks, and redundant service managers. Prefer event-driven system APIs and on-demand activation. Before a production release, publish measurements and budgets for login/startup time, idle and interaction CPU, resident memory, input latency, and component overhead on representative low-end hardware.

## Session safety

LinWin must coexist with the installed desktop until the user explicitly selects LinWin as the session shell. Preserve the previous session as a recovery option, provide an easy way back, and test upgrade, failed-start fallback, rollback, and uninstall on every supported desktop/session combination.
