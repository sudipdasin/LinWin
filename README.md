# LinWin

LinWin is an open-source, lightweight, all-in-one Linux desktop shell for people moving from Windows. Its goal is to make Linux feel familiar and easy from the first login, including for people who do not know Linux, terminals, or Bash.

LinWin brings the taskbar, Start-style application menu, Alt+Tab window switching, file workflows, clipboard, Bluetooth, accessibility, task manager, and system settings into one coherent shell. It replaces the desktop-shell layer while retaining the host distribution, kernel, drivers, and applications. The shared shell is being developed in C++20 and Qt 6; supported components are enabled by default and can be turned off individually. The launcher reads the Linux desktop application registry and remembers pinned apps.

## Project status

**Early development.** The current build is a shell-interface preview with a persistent in-window bottom panel, a searchable Start popup, system overview, and opt-in clipboard component; it is not yet the complete login-session shell or graphical installer. The planned installer will detect the distribution, desktop session, kernel, package manager, and available hardware, then select compatible packages and configure LinWin without asking users to use a terminal. The finished shell aims to handle normal setup and daily tasks entirely through a clear graphical interface. The prototype clipboard keeps up to 50 text clips while open; capture is off by default and saved text is unencrypted. Actual desktop-panel replacement, Alt+Tab/window tracking, and session integration are still future work. See [the architecture](docs/ARCHITECTURE.md), [installer specification](docs/INSTALLER_SPEC.md), and [roadmap](ROADMAP.md).

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

Run from the build directory with `./linwin`. Open **Start** on the bottom panel to search installed applications, pin favorites, and launch apps through the host desktop registry. Use **Home** and **Clipboard** to switch pages. In Clipboard, enable history if desired and copy text while LinWin is open. Select an entry and double-click it or choose **Copy selected** to restore it. Use **Remove** or **Clear history** to erase saved items. Package formats can be generated with CPack where the corresponding packaging tools are installed:

```sh
cd build
cpack -G DEB
cpack -G RPM
```

The DEB and RPM package generators are starting points; maintainers must validate dependencies and install/uninstall behaviour on each target distribution before calling a package supported.

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
