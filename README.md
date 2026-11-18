# LinWin

LinWin is an open-source, lightweight, all-in-one Linux desktop shell for people moving from Windows. Its goal is to make Linux feel familiar and easy from the first login, including for people who do not know Linux, terminals, or Bash.

LinWin is being built as one familiar shell for Linux, using C++20 and Qt 6 while keeping the host distribution, kernel, drivers, and applications. Its goal is to bring the taskbar, Start-style app menu, window switching, file workflows, clipboard, device controls, task manager, and system settings into one easy interface.

## Project status

**Status: pre-beta engineering preview.** LinWin currently runs as a regular application window; it is not a login-session shell replacement and does not include the graphical installer, recovery flow, or component setup wizard. Do not replace your normal desktop with this build. For evaluation, run it in a test account or virtual machine. Current pages include Start app search and pinned apps, a basic file manager, a /proc process monitor, a searchable native Settings launcher, and opt-in text clipboard history. X11 taskbar window listing and in-app Alt+Tab require wmctrl; Wayland continues to use the host compositor's switcher. Host settings and hardware support depend on installed desktop tools and services. The clipboard preview stores text unencrypted, so capture is off by default. See [beta readiness and testing limits](docs/BETA_READINESS.md), [the architecture](docs/ARCHITECTURE.md), [installer specification](docs/INSTALLER_SPEC.md), and [roadmap](ROADMAP.md).

## Intended compatibility

LinWin aims to replace the shell layer on multiple distributions and desktop environments, including:

- Linux Mint Cinnamon, Xfce, and MATE
- Ubuntu with KDE Plasma
- Fedora with KDE Plasma or Xfce
- Debian and other distributions as adapters and packages mature

Compatibility will be published per distribution, desktop, session type, and shell component. Desktop-specific adapters will integrate with the host compositor and session; LinWin will not claim universal support where an interface is unavailable.

## Build from source (contributors)

These commands are for developers building the prototype from source. They are not the planned end-user installation flow; the finished product is intended to install and configure through the graphical wizard.

Requirements:

- CMake 3.21 or newer
- A C++20 compiler
- Qt 6.4 or newer, Widgets module
- GLib/GIO Unix development files (`gio-unix-2.0`)

On Debian/Ubuntu/Mint, install the compiler, CMake, and Qt 6 development packages, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run from the build directory with `./linwin`. Open **Start** on the bottom panel to search installed applications, pin favorites, and launch apps through the host desktop registry. Use **Home**, **Files**, **Tasks**, **Clipboard**, and **Settings** to switch pages. Press **Ctrl+,** to open Settings and search for a host settings panel. In Files, double-click folders to browse, use the action buttons or standard shortcuts (Ctrl+C/X/V, F2, Delete) for common file tasks, and drag items into folders. In Tasks, search or sort the process list, review CPU and memory use, choose **End task** to request a normal close, or use **Force stop** only when necessary. In Clipboard, enable history if desired and copy text while LinWin is open. Select an entry and double-click it or choose **Copy selected** to restore it. Use **Remove** or **Clear history** to erase saved items. A generic preview archive can be generated with CPack. It is a developer/test package, not an end-user installer:

```sh
cd build
cpack -G TGZ
```

Pull requests produce short-lived per-distro archive artifacts after compiling, testing, and checking the staged install tree. These artifacts do not configure a login session or install a shell replacement. Native DEB/RPM packages, dependency resolution, upgrades, rollback, and uninstall must be validated before a public beta install is offered.

## Design principles

- Use the host Linux kernel, drivers, and system services; detect compatibility and configure the right packages automatically.
- Access hardware and kernel-managed state through documented interfaces and system services. Keep the shell unprivileged and use narrow, user-authorized helpers only when elevated access is required.
- Keep the core small. Avoid duplicate daemons, bundled applications, and always-running services for optional features; load components on demand where practical.
- Enable the standard component set by default, and let the installation wizard and settings hub turn each component off independently.
- Make normal setup and everyday use possible through the GUI, with no terminal or Bash knowledge required.
- Preserve the existing desktop as a recovery path. Explain the session change and make switching back or uninstalling straightforward.
- Publish compatibility per distribution, desktop, session type, and component.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md), [ROADMAP.md](ROADMAP.md), [the architecture](docs/ARCHITECTURE.md), [the installer specification](docs/INSTALLER_SPEC.md), [the usability specification](docs/USABILITY_SPEC.md), and [SECURITY.md](SECURITY.md) before opening a change. Bug reports should state the distribution, version, desktop environment, display server, and steps to reproduce. Please remove personal paths, hostnames, usernames, and device serials from logs before sharing them.

## License

LinWin is licensed under the GNU General Public License, version 3 or (at your option) any later version. See [LICENSE](LICENSE).
