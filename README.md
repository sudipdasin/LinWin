<!-- LATEST-RELEASE:START -->
## Latest published release: v1.0.0-beta.8
**[Release notes and downloads](https://github.com/sudipdasin/LinWin/releases/tag/v1.0.0-beta.8)** · Prerelease

| Asset | Download |
| --- | --- |
| Source archive | [Download ZIP](https://github.com/sudipdasin/LinWin/releases/tag/v1.0.0-beta.8) |
<!-- LATEST-RELEASE:END -->

# LinWin

LinWin is an open-source, lightweight, all-in-one Linux desktop shell for people moving from Windows. Its goal is to make Linux feel familiar and easy from the first login, including for people who do not know Linux, terminals, or Bash.

LinWin is being built as one familiar shell for Linux, using C++20 and Qt 6 while keeping the host distribution, kernel, drivers, and applications. Its goal is to bring the taskbar, Start-style app menu, window switching, file workflows, clipboard, device controls, task manager, and system settings into one easy interface.

## Free and Pro development

The public LinWin repository is the complete, independently useful **LinWin Free** shell and is licensed under GPL-3.0-or-later. Paid development will live in the separate, private [`LinWin-Pro`](https://github.com/sudipdasin/LinWin-Pro) repository. Its `public-core/` directory is maintained as a synchronized snapshot of this repository; all proprietary implementation, assets, and release material stay in that private repository. LinWin Free remains installable and usable without LinWin Pro. The Pro product is planned as a separately installed companion application communicating through documented, process-separated interfaces; the public project will not load proprietary libraries into the GPL shell. See [commercial edition strategy](docs/COMMERCIAL_EDITION_STRATEGY.md).

## Project status

**Status: preparing 1.0.0-beta.8, an experimental public testing preview.** LinWin now launches by default as a borderless full-screen overlay above the current desktop, with a desktop workspace and LinWin taskbar. Use **Linux desktop** or **Meta+D** to minimize LinWin and reveal the existing desktop, then select LinWin from the host taskbar to return; use **×** or **More → Close LinWin** to exit. Run `linwin --window` for a normal development window. This overlay is an application-level approximation and does not replace the Linux display manager, window manager, or system shell. The optional X11 login-session adapter remains available through LinWin Setup on systems with XFCE's `xfwm4`, `xfsettingsd`, and `xfdesktop`; your original desktop remains selectable as a recovery option. The adapter does not support Wayland or other window managers yet. Clipboard history is opt-in and saved text is unencrypted. X11 window listing and in-app Alt+Tab require `wmctrl`; Wayland uses the host compositor's switcher. See [beta readiness and testing limits](docs/BETA_READINESS.md), [architecture](docs/ARCHITECTURE.md), [installer specification](docs/INSTALLER_SPEC.md), and [roadmap](ROADMAP.md).

The taskbar adapts to narrow windows: its secondary actions move into **More**, and open-window/system details collapse to make room for the core navigation. On X11, the **Desktop** menu lists the current virtual desktop, switches to another desktop, and adds one without a terminal; the menu also appears under **More** on narrow windows. LinWin reads these workspaces through `wmctrl`; Wayland keeps its compositor-managed workspace controls. The Files page adds a Quick access sidebar for common folders, per-user pinned folders, mounted removable media, and a name filter for the current folder.

## Intended compatibility

LinWin aims to replace the shell layer on multiple distributions and desktop environments, including:

- Linux Mint Cinnamon, Xfce, and MATE
- Ubuntu with KDE Plasma
- Fedora with KDE Plasma or Xfce
- Debian and other distributions as adapters and packages mature

The first installable shell session targets X11 with XFCE session tools on Ubuntu/Debian-family systems, Fedora, and Arch x86_64. Mint Xfce can use the Ubuntu/Debian package when dependencies are available. The other listed distributions and desktops remain goals; package availability alone does not mean their login session is supported. Wayland support requires compositor-specific adapters and is not included in this preview.

## Install and test on XFCE

Download the package for your Linux distribution. These files install the LinWin application and its launcher; Linux executables do not use the Windows `.exe` extension.

| Distribution | Download |
| --- | --- |
| Ubuntu 24.04, Ubuntu-based Linux Mint | [Download the Ubuntu .deb](https://github.com/sudipdasin/LinWin/releases/download/v1.0.0-beta.8/linwin-1.0.0-beta.8-ubuntu-24.04.deb) |
| Debian 13 | [Download the Debian .deb](https://github.com/sudipdasin/LinWin/releases/download/v1.0.0-beta.8/linwin-1.0.0-beta.8-debian-13.deb) |
| Fedora | [Download the Fedora .rpm](https://github.com/sudipdasin/LinWin/releases/download/v1.0.0-beta.8/linwin-1.0.0-beta.8-fedora.rpm) |
| Arch Linux x86_64 | [Download the Arch package](https://github.com/sudipdasin/LinWin/releases/download/v1.0.0-beta.8/linwin-1.0.0-beta.8-arch-x86_64.pkg.tar.zst) |
| Any supported package above | [Download the all-packages ZIP](https://github.com/sudipdasin/LinWin/releases/download/v1.0.0-beta.8/LinWin-1.0.0-beta.8-linux-packages.zip) |

To test the shell session, install the package using your graphical software installer, open **LinWin Setup**, choose **Install LinWin session**, then log out and select **LinWin (X11)** from your login screen's session menu. Your existing XFCE session remains available so you can switch back. The setup wizard can offer the required XFCE session packages when they are missing.

Use the [release page](https://github.com/sudipdasin/LinWin/releases) for the current beta and its checksums. The download list at the top of this page updates when a new release is published.

## Install the beta application preview

Download the package for your distribution from the [latest LinWin prerelease](https://github.com/sudipdasin/LinWin/releases):

- Ubuntu, Debian, and Linux Mint: download the **.deb** package, then open it in your Software application and choose Install.
- Fedora: download the **.rpm** package, then open it in GNOME Software or KDE Discover and choose Install.
- Arch Linux x86_64: download the **.pkg.tar.zst** package and install it with your package manager (for example, pacman or a graphical pacman front end).

After installation, open **LinWin Setup** from the applications menu. If XFCE session components are missing on Debian/Ubuntu/Mint, Fedora/RHEL, or Arch-family systems, the wizard offers to install the required base packages through the detected native package manager. It displays the exact package list and asks you to confirm before the system authorization prompt; the package manager may also install required dependencies. On other distributions, use the graphical software manager to install XFCE first. Then choose **Install LinWin session**. The wizard adds only the selectable session entry and does not replace your current desktop. Log out and select **LinWin** in the login screen's session menu. Use **Sign out** on the LinWin taskbar to return to the login screen, then choose your previous session. To uninstall, open **Uninstall LinWin** from the applications menu and confirm package removal. The same checksum file can verify the downloaded package. In LinWin Settings, switch Start, Files, Task Manager, or Clipboard off to remove that feature from the taskbar; these choices are saved for your account and default to on. Keyboard shortcuts include **Super** or **Ctrl+Esc** for Start, **Super+E** for Files, **Ctrl+Shift+Esc** for Task Manager, **Ctrl+,** for Settings, and **Ctrl+Plus / Ctrl+Minus / Ctrl+0** to enlarge, reduce, or reset LinWin text size. **Super+V** or **Ctrl+Alt+V** opens a searchable clipboard picker while LinWin is active; choose a clip, then paste with **Ctrl+V**. The taskbar desktop menu can switch among X11 virtual desktops or add another workspace when `wmctrl` is installed.

The ordinary **LinWin** application launcher opens the overlay over your current desktop. Select **Linux desktop** or press **Meta+D** to minimize it and see the host desktop; selecting LinWin in the host taskbar restores the overlay. **×** or **More → Close LinWin** closes it. This is an application overlay, not a replacement for the host's window manager or display manager. `linwin --window` opens a decorated window for development.

In **Files**, open common folders from the left sidebar, pin the folder you are viewing for next time, or type in **Search this folder** to filter by name. Mounted media under the standard Linux mount locations appears under **Devices** when available.

This is an experimental shell-session preview, not a production-ready desktop replacement. It starts the XFCE window manager/settings/desktop alongside LinWin's taskbar, Start menu, file manager, task manager, clipboard, and settings hub. The setup wizard installs only the required XFCE session packages on the listed package-manager families; it does not yet install optional component dependencies, provide full transactional rollback, or support Wayland session adapters. The Task Manager's **Experimental** tab groups network, disk, Bluetooth, startup-program, service, and GPU views; these may be incomplete depending on hardware, services, permissions, and desktop setup. Clipboard history is opt-in and saved text is stored locally without encryption. Middle-click paste is off by default in LinWin text fields to prevent accidental insertion; the preference does not change other apps.

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

Read [CONTRIBUTING.md](CONTRIBUTING.md), [ROADMAP.md](ROADMAP.md), [the architecture](docs/ARCHITECTURE.md), [commercial edition strategy](docs/COMMERCIAL_EDITION_STRATEGY.md), [the installer specification](docs/INSTALLER_SPEC.md), [the usability specification](docs/USABILITY_SPEC.md), and [SECURITY.md](SECURITY.md) before opening a change. Bug reports should state the distribution, version, desktop environment, display server, and steps to reproduce. Please remove personal paths, hostnames, usernames, and device serials from logs before sharing them.

## License

LinWin is licensed under the GNU General Public License, version 3 or (at your option) any later version. See [LICENSE](LICENSE).
