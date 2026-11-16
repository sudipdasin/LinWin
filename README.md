# LinWin

LinWin is an open-source, lightweight Linux desktop-shell replacement designed to make everyday desktop use approachable, responsive, and consistent across distributions. It is intended for people moving from Windows as well as existing Linux users.

LinWin replaces the desktop-shell layer, such as the panel, launcher, overview, and selected system controls, while retaining the host distribution, Linux kernel, drivers, and applications. It is not a new distribution, kernel, or bundle of unrelated utilities. The shared shell is being developed in C++20 and Qt 6, with optional components that can be enabled or disabled independently.

## Project status

**Early development.** The current build is a prototype dashboard plus an opt-in clipboard component. It is not yet a full shell replacement and does not include the installation wizard. Clipboard history captures up to 50 text clips while LinWin is open; capture is off by default and saved text is unencrypted. See [the architecture](docs/ARCHITECTURE.md), [installer specification](docs/INSTALLER_SPEC.md), and [roadmap](ROADMAP.md) for the intended design and stages.

## Intended compatibility

LinWin aims to replace the shell layer on multiple distributions and desktop environments, including:

- Linux Mint Cinnamon, Xfce, and MATE
- Ubuntu with KDE Plasma
- Fedora with KDE Plasma or Xfce
- Debian and other distributions as adapters and packages mature

Compatibility will be published per distribution, desktop, session type, and shell component. Desktop-specific adapters will integrate with the host compositor and session; LinWin will not claim universal support where an interface is unavailable.

## Build

Requirements:

- CMake 3.21 or newer
- A C++20 compiler
- Qt 6.4 or newer, Widgets module

On Debian/Ubuntu/Mint, install the compiler, CMake, and Qt 6 development packages, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run from the build directory with `./linwin`. Open **Clipboard**, enable history if desired, and copy text while LinWin is open. Select an entry and double-click it or choose **Copy selected** to restore it. Use **Remove** or **Clear history** to erase saved items. Package formats can be generated with CPack where the corresponding packaging tools are installed:

```sh
cd build
cpack -G DEB
cpack -G RPM
```

The DEB and RPM package generators are starting points; maintainers must validate dependencies and install/uninstall behaviour on each target distribution before calling a package supported.

## Design principles

- Use the host Linux kernel, drivers, and system services; do not fork or replace them.
- Access hardware and kernel-managed state through documented interfaces and system services. Keep the shell unprivileged and use narrow, user-authorized helpers only when elevated access is required.
- Keep the core small. Avoid duplicate daemons, bundled applications, and always-running services for optional features; load components on demand where practical.
- Enable the standard component set by default, and let the installation wizard and settings hub turn each component off independently.
- Preserve the existing desktop as a recovery path. Explain the session change and make switching back or uninstalling straightforward.
- Publish compatibility per distribution, desktop, session type, and component.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md), [ROADMAP.md](ROADMAP.md), [the architecture](docs/ARCHITECTURE.md), [the installer specification](docs/INSTALLER_SPEC.md), [the usability specification](docs/USABILITY_SPEC.md), and [SECURITY.md](SECURITY.md) before opening a change. Bug reports should state the distribution, version, desktop environment, display server, and steps to reproduce. Please remove personal paths, hostnames, usernames, and device serials from logs before sharing them.

## License

LinWin is licensed under the GNU General Public License, version 3 or (at your option) any later version. See [LICENSE](LICENSE).
