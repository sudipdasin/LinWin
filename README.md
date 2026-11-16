# LinWin

LinWin is an open-source project to make everyday Linux desktop interactions easier and more consistent across distributions and desktop environments.

The project is being built as a set of installable tools and desktop integrations. It does not require a custom Linux kernel or a single base distribution. The initial implementation uses C++20 and Qt 6.

## Project status

**Early development.** The dashboard reports the detected distribution, desktop session, and kernel. The Clipboard tab includes opt-in text history with search, copy, remove, and clear actions. Capture is off by default, stores up to 50 recent text items in the user's local Qt application settings, and works only while LinWin is open. Saved text is not encrypted. Clipboard image/file history, background capture, file management, window switching, taskbar/launcher integrations, and Bluetooth workflows remain planned.

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

Run from the build directory with `./linwin`. Open **Clipboard**, enable history if desired, and copy text while LinWin is open. Select an entry and double-click it or choose **Copy selected** to restore it. Use **Remove** or **Clear history** to erase saved items. Package formats can be generated with CPack where the corresponding packaging tools are installed:

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
