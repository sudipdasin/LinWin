# LinWin architecture

LinWin is a lightweight, all-in-one desktop-shell replacement for Linux users who want familiar Windows-style workflows without needing to learn Linux, terminals, or Bash. It owns the main shell experience while continuing to use the host distribution's kernel, drivers, services, applications, and package manager.

## Product experience

Bring common desktop actions together in one consistent graphical shell: a taskbar, Start-style launcher, Alt+Tab switching, file operations, clipboard, Bluetooth, accessibility, task manager, and system settings. The current launcher prototype searches the host desktop registry and delegates launch behavior to GIO instead of parsing `Exec` fields itself. Favor familiar labels and visible actions. Normal setup and daily use must not require terminal commands or shell scripts. Keep advanced controls available without making beginners depend on them.

## Layers

1. **Shell core:** one C++20/Qt 6 session process for the panel, launcher, overview, notifications, shortcuts, and shell settings. Keep this process small and responsive.
2. **Optional components:** clipboard, file workflows, Bluetooth, device controls, task manager, and accessibility features. Ship them in the standard install profile enabled by default, but let users disable each independently. Load or activate them only when practical; do not create a permanent daemon for every feature.
3. **Desktop/session adapters:** connect to the active X11 or Wayland session and its supported window manager/compositor interfaces. Adapters declare their compatibility; the shared shell must not assume that one desktop's private APIs work everywhere.
4. **Host-system interfaces:** detect the host distribution, release, CPU architecture, kernel, desktop session, package manager, hardware, and available services. Use existing kernel drivers and documented Linux userspace interfaces, system buses, portals, and services. Examples include udev for device discovery, BlueZ for Bluetooth, and the host's network, audio, power, and login services.
5. **Narrow privileged helper:** only add one for an operation that genuinely requires elevated privileges. Keep the UI and normal shell process unprivileged, authenticate through the host authorization mechanism, limit each helper action, and report the result.

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
