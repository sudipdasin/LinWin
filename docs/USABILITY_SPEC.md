# LinWin usability specification

This document turns the project goal into observable user behaviour. The clipboard section distinguishes the current preview from the full desired behaviour; other items remain goals unless explicitly marked as implemented.

## Current implementation

- The Overview page displays detected distribution, desktop environment, session type, and kernel.
- The Clipboard page provides opt-in text capture while LinWin is open, search, copy, remove, and clear actions, and a 50-item history stored in local Qt application settings.
- Clipboard capture defaults to off. Saved clipboard text is not encrypted. Image/file clips and background capture are not supported yet; desktop compositor permissions may limit access.

## Product goals

- Make familiar desktop tasks discoverable and consistent for people switching to Linux.
- Run on multiple distributions and desktop environments without requiring a LinWin-specific kernel or base OS.
- Preserve each user's ability to keep the host desktop's normal tools and settings.
- Make missing capabilities clear, especially when a desktop environment or system service does not expose a needed interface.

## Cross-desktop behaviour

Each feature must declare whether it is a shared application, a system-service integration, or a desktop-specific adapter. Desktop-specific behaviour must not be represented as universally supported. The support matrix should record distribution, desktop, session type (Wayland/X11), version, and verification status.

## Interaction requirements

### Window and application switching

- `Alt+Tab` presents recognizable previews and cycles through open windows in a stable order.
- Switching works across multiple monitors and virtual desktops, with the current desktop/window indicated.
- A desktop adapter must use the host window manager's supported mechanisms and avoid fighting its own shortcut bindings.

### Taskbar and application launcher

- Show pinned and running apps distinctly; group related windows and provide window previews.
- Right-click exposes useful app actions where the desktop or app supports them.
- The launcher searches installed applications and exposes pinned apps, recent items, settings, and power actions.
- Replacing or adding a panel/launcher is opt-in and reversible.

### File operations and drag-and-drop

- Rename selects the base filename by default and leaves the extension unselected when the file has one.
- Copy, cut, paste, and rename work through keyboard shortcuts and clearly labelled context-menu actions.
- Drag-and-drop communicates copy, move, link, open, or pin before completing an ambiguous operation.
- File operations show progress and conflicts; offer undo when the underlying operation can be reversed safely.
- Shortcuts are distinct from copies and links, have a clear context-menu action, and can be removed without deleting the original.

### Clipboard and pointer behaviour

- Implemented preview: opt-in text history with search, copy, removal, clear-history, and a bounded 50-item local store.
- Remaining: pinned items, image/file content where supported, background capture adapters, and finer privacy controls.
- Explicit `Ctrl+C`/`Ctrl+V` clipboard use is the default mental model.
- Middle-click selection paste is a visible, opt-in preference; changing it must not disable middle-click link/tab actions in applications.
- Clipboard history must handle text and common image/file content where the compositor permits it, and explain unavailable formats.

### Zoom and accessibility

- Offer display scaling and text-size controls with clear per-display scope where supported.
- Provide a magnifier and keyboard-accessible controls.
- Layouts remain usable at narrow widths and common scaling levels; text must not be clipped or rely on colour alone.

### Bluetooth

- Show nearby devices with recognizable names and device categories.
- Pairing explains PIN/passkey prompts and reports success or a useful failure reason.
- Paired devices can be connected, disconnected, and removed without repeating discovery unless required.
- Audio devices expose input/output selection where supported by the host audio stack.
- Use the system Bluetooth service; do not implement a parallel radio manager.

### Task manager

- Present applications and processes with CPU and memory use, sort/search, and a clear close-app action.
- Make force-quit available for unresponsive apps, with an explicit confirmation when data loss is possible.
- Put services, startup items, and advanced process controls behind an advanced view and the host's authorization mechanisms.

### System configuration

- Provide one searchable entry point for LinWin settings and links into native display, sound, network, printer, account, power, update, privacy, and accessibility panels.
- Prefer invoking the desktop's existing settings module over writing configuration files directly.
- Privileged changes are narrow, authenticated, and reported back to the user.

## Initial delivery order

1. Shared diagnostics and support matrix.
2. Clipboard history and preferences, with distinct desktop/compositor adapters.
3. File operation helper and shortcut handling.
4. Searchable settings hub and Bluetooth pairing flow.
5. Desktop adapters for launchers, taskbars, and window switching.
6. Task manager and deeper system configuration links.

The order may change as maintainers learn which integration APIs are reliable on the target desktops.
