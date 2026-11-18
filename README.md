# LinWin

LinWin is an open-source, lightweight, all-in-one Linux desktop shell for people moving from Windows. Its goal is to make Linux feel familiar and easy from the first login, including for people who do not know Linux, terminals, or Bash.

LinWin is being built as one familiar shell for Linux, using C++20 and Qt 6 while keeping the host distribution, kernel, drivers, and applications. Its goal is to bring the taskbar, Start-style app menu, window switching, file workflows, clipboard, device controls, task manager, and system settings into one easy interface.

## Project status

**Status: installable beta application preview.** LinWin installs as a regular desktop application, with a desktop-menu entry; it does not replace your login session or include the graphical shell installer, session recovery, or component setup wizard. You can install it, open its Start-style launcher, file manager, task manager, clipboard, Settings Hub, and experimental resource/device panels, then remove it with your distro's package manager. Test in a virtual machine or test account. Do not select it as your only desktop. Clipboard history is opt-in and saved text is unencrypted. X11 window listing and in-app Alt+Tab require wmctrl; Wayland uses the host compositor's switcher. See [beta readiness and testing limits](docs/BETA_READINESS.md), [architecture](docs/ARCHITECTURE.md), [installer specification](docs/INSTALLER_SPEC.md), and [roadmap](ROADMAP.md).

## Intended compatibility

LinWin aims to replace the shell layer on multiple distributions and desktop environments, including:

- Linux Mint Cinnamon, Xfce, and MATE
- Ubuntu with KDE Plasma
- Fedora with KDE Plasma or Xfce
- Debian and other distributions as adapters and packages mature

Compatibility will be published per distribution, desktop, session type, and shell component. Desktop-specific adapters will integrate with the host compositor and session; LinWin will not claim universal support where an interface is unavailable.

## Install the beta application preview

Download the package for your distribution from the [latest LinWin prerelease](https://github.com/sudipdasin/LinWin/releases):

- Ubuntu, Debian, and Linux Mint: download the **.deb** package, then open it in your Software application and choose Install.
- Fedora: download the **.rpm** package, then open it in GNOME Software or KDE Discover and choose Install.
- Arch Linux x86_64: download the **.pkg.tar.zst** package and install it with your package manager (for example, pacman or a graphical pacman front end).

After installation, launch **LinWin** from the applications menu. Uninstall it using the same software/package manager. These packages install the app and desktop-menu entry only; they do not change your login session, replace your taskbar, or install a shell session. The archive's checksum file can be used to verify the downloaded package.

The release is an experimental feature preview, not a supported system-shell replacement. The Task Manager's **Experimental** tab groups network, disk, Bluetooth, startup-program, service, and GPU views. They may be unavailable or incomplete depending on hardware, services, permissions, and desktop setup.

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

Run from the build directory with `./linwin`. Open **Start** on the bottom panel to search installed applications, pin favorites, and launch apps through the host desktop registry. Use **Home**, **Files**, **Tasks**, **Clipboard**, and **Settings** to switch pages. In Tasks, stable process/performance views stay at the top level; network, disk, Bluetooth, startup, services, and GPU controls are grouped under **Experimental**. Press **Ctrl+,** to open Settings and search for a host settings panel. In Files, double-click folders to browse, use the action buttons or standard shortcuts (Ctrl+C/X/V, F2, Delete) for common file tasks, and drag items into folders. In Tasks, search or sort the process list, review CPU and memory use, choose **End task** to request a normal close, or use **Force stop** only when necessary. In Clipboard, enable history if desired and copy text while LinWin is open. Select an entry and double-click it or choose **Copy selected** to restore it. Use **Remove** or **Clear history** to erase saved items. For a generic developer archive, use CPack. The public beta packages are generated and install-tested in CI:

```sh
cd build
cpack -G TGZ
```

CI builds and tests DEB packages for Ubuntu/Debian, an RPM for Fedora, and an Arch package for x86_64; it installs and removes each package in a clean distro container. This verifies package-manager integration and files, not real desktop-session operation or rollback/upgrade behavior.

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
