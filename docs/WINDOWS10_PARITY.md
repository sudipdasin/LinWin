# Windows 10 interaction parity — product acceptance contract

LinWin is an independent Linux desktop, not a Microsoft product. Use original artwork, names and code; no bundled Windows fonts, icons, wallpaper or binaries without redistribution rights. Functional familiarity is the goal, not a claim of Windows compatibility.

## Delivery gates

1. Overlay: real desktop items, single-select/double-open, multi-selection, right-click, compact taskbar, Start search, pinned apps; preserve exit and recovery. No automatic execution of downloaded files.
2. Window architecture: independent movable/resizable native tool windows; desktop must stay behind them; X11 EWMH adapter, task grouping, activation, show desktop, snap, preview, restore; separate compositor-specific Wayland adapter.
3. Explorer: This PC, removable drives, network, Recycle Bin, copy/move conflicts, progress and cancellation, undo, shortcuts, rich properties, indexed search, safe drag/drop.
4. Input: Ctrl/C/X/V/Z, explicit clipboard, Win+V with text/image pinning and opt-in retention, middle-click navigation/autoscroll where supported, configurable application-specific behavior; never suppress middle-button events globally without a reversible adapter.
5. System: notification center, settings, power, lock, display/audio/network/Bluetooth, printers, USB and storage hotplug, device status and helpful driver diagnostics, package discovery, updates, authorized helpers.
6. Native session: independent XFCE/X11 login option with recovery, eventually compositor integrations. Do not replace a working login session until real VM tests pass. Maintain distro and hardware support matrix.
7. Release: no GitHub Actions or CircleCI. Run local or approved independent builds and CTest, X11 desktop integration, Mint 22.3 XFCE VM install/upgrade/rollback, and visual/interaction review before merging or publishing. Secure updates must verify signatures, compatibility and rollback.

## Acceptance

Each feature needs explicit interaction scenarios, negative-path tests, capability detection and fallback. No mock buttons or fake telemetry. Unsupported device drivers, proprietary Windows applications and Windows-only kernel APIs are outside shell control; disclose these boundaries and provide native alternatives where feasible. Do not mark parity complete based on visual similarity or offscreen tests alone.

## Initial implementation in this branch

Real desktop directory entries and internal shortcuts in an icon view; double-click or Enter opens, single selection does not open, F5 and directory change refresh; desktop context menu and compact flat taskbar; Start panel anchors to lower-left. External items use host handlers and executable non-directories are not launched directly. Icon drag/move, actual trash/This PC, floating tool windows and system-level hardware controls remain subsequent work. The source is a candidate, not a validated binary.
