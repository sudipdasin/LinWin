# LinWin

LinWin is an open-source project to make everyday Linux desktop interactions easier and more consistent across distributions and desktop environments.

The project is being built as a set of installable tools and desktop integrations. It does not require a custom Linux kernel or a single base distribution. The initial implementation uses C++20 and Qt 6.

## Project status

**Early development.** This repository contains the first buildable project scaffold and a system-profile dashboard. The dashboard reports the detected distribution, desktop session, and kernel. Clipboard history, file management, window switching, taskbar/launcher integrations, Bluetooth workflows, and other usability features are planned; they are not implemented by this initial scaffold.

## Intended compatibility

LinWin aims to support multiple distributions and desktops, including:

- Linux Mint Cinnamon, Xfce, and MATE
- Ubuntu with KDE Plasma
- Fedora with KDE Plasma or Xfce
- Debian and other distributions as adapters and packages mature

Compatibility will be published per feature and per desktop environment. A desktop panel or window-switcher integration may need a desktop-specific adapter, while standalone tools can be shared.

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

Run from the build directory with `./linwin`. Package formats can be generated with CPack where the corresponding packaging tools are installed:

```sh
cd build
cpack -G DEB
cpack -G RPM
```

The DEB and RPM package generators are starting points; maintainers must validate dependencies and install/uninstall behaviour on each target distribution before calling a package supported.

## Design principles

- Keep the shared core independent of a particular distribution.
- Prefer documented desktop and system interfaces over shelling out to distribution-specific commands.
- Keep the user interface unprivileged; request authorization only for specific operations that require it.
- Add desktop-specific adapters for panel, launcher, window-switching, and settings integration where no common interface exists.
- Make compatibility and limitations visible instead of silently degrading behaviour.
- Keep features optional and removable; do not replace the user's desktop without an explicit choice.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md), [ROADMAP.md](ROADMAP.md), [the usability specification](docs/USABILITY_SPEC.md), and [SECURITY.md](SECURITY.md) before opening a change. Bug reports should state the distribution, version, desktop environment, display server, and steps to reproduce. Please remove personal paths, hostnames, usernames, and device serials from logs before sharing them.

## License

LinWin is licensed under the GNU General Public License, version 3 or (at your option) any later version. See [LICENSE](LICENSE).
