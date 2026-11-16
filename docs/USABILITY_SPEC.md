# LinWin usability specification

This document turns the project goal into observable user behaviour. LinWin is intended to replace the desktop-shell layer, not the host distribution or kernel. The clipboard section distinguishes the current preview from the full desired behaviour; other items remain goals unless explicitly marked as implemented.

## Current implementation

- The Home page displays detected distribution, desktop environment, session type, and kernel.
- The in-window bottom panel provides Home, Files, Tasks, and Clipboard navigation, a detected system/session label, and a live clock/date display.
- The Start popup searches launchable desktop applications, opens them through GIO, focuses search when opened, and stores pinned app IDs in local Qt application settings.
- The Files page browses from the home folder, supports location entry and back/up/home navigation, opens files with the host default application, and provides new folder, rename, copy/cut/paste, shortcut-link, and move-to-Trash actions.
- The Files page supports common keyboard shortcuts (Ctrl+C/X/V, F2, Delete, Ctrl+L, Alt+Up), multi-selection, context menus, and Qt file-view drag-and-drop.
- The Tasks page lists visible processes with CPU share, resident memory, PID, owner, and state; it searches, sorts, and refreshes every two seconds using Linux `/proc` while the page is open. Polling pauses when the user leaves the page.
- **End task** sends SIGTERM after confirmation; **Force stop** sends SIGKILL after a separate data-loss warning. Both actions are limited to the current user's processes, recheck process identity before signalling, and do not request elevated privileges.
- The Clipboard page provides opt-in text capture while LinWin is open, search, copy, remove, and clear actions, and a 50-item history stored in local Qt application settings.
- Clipboard capture defaults to off. Saved clipboard text is not encrypted. Image/file clips and background capture are not supported yet; desktop compositor permissions may limit access.
- LinWin does not yet launch as a login-session shell, manage hardware, provide the installation wizard, replace the host taskbar, track running windows, or control Alt+Tab/window switching.
- The file manager is an initial preview: large copies can take time, cross-device cut/move may fail with an explanation, and operation progress, undo, advanced conflict handling, and broad compositor testing remain future work.
- The task manager is an initial process view, not a live-window taskbar: desktop app/window grouping, per-process history, services, startup management, and authorized control of other users' system processes are not implemented.

## Product goals

- Make Windows switchers feel at home with familiar conventions such as a taskbar, Start-style app menu, Alt+Tab, direct file actions, and clear system controls.
- Make normal installation, configuration, and daily computer use possible entirely through a graphical interface; users should not need Linux, terminal, or Bash knowledge.
- Integrate the common desktop features into one shell instead of requiring newcomers to assemble separate tools.
- Replace the user's desktop-shell layer across supported distributions and desktop environments without requiring a LinWin-specific kernel or base OS.
- Keep the host kernel, drivers, applications, and package manager in place; expose controls through supported Linux userspace interfaces.
- Keep the shell lean and responsive; avoid unnecessary always-running processes and duplicate system services.
- Enable the standard component set by default while allowing each component to be switched off in setup and later settings.
- Preserve each user's ability to keep the host desktop's normal tools and settings.
- Make missing capabilities clear, especially when a desktop environment or system service does not expose a needed interface.

## Cross-desktop behaviour

The installer must identify the host distribution, kernel, package manager, and desktop session, then choose compatible packages and adapters automatically where supported. It must surface unsupported combinations in plain language. Each feature must declare whether it is a core shell function, optional component, host-system integration, or desktop/session adapter. Desktop-specific behaviour must not be represented as universally supported. The support matrix should record distribution, desktop, session type (Wayland/X11), version, and verification status.

## Beginner experience

- Provide one graphical place for installation, component selection, system settings, and recovery.
- Use familiar names, visible controls, and helpful explanations instead of requiring commands or scripts.
- Keep advanced options discoverable but out of the way until requested.
- Show the detected Linux system and explain compatibility without making users understand distro or kernel terminology.

## Interaction requirements

### Window and application switching

- `Alt+Tab` presents recognizable previews and cycles through open windows in a stable order.
- Switching works across multiple monitors and virtual desktops, with the current desktop/window indicated.
- A desktop adapter must use the host window manager's supported mechanisms and avoid fighting its own shortcut bindings.

### Taskbar and application launcher

- Launcher preview: open the in-window Start popup from the bottom panel, search installed apps, open them using the host application registry, and pin/unpin favorites.
- Remaining: show pinned and running apps distinctly; group related windows and provide window previews.
- Right-click exposes useful app actions where the desktop or app supports them.
- The launcher searches installed applications and exposes pinned apps, recent items, settings, and power actions.
- Pinned and running apps are distinct; related windows can be grouped and previewed.
- Right-click exposes useful app actions where the desktop or app supports them.
- Search finds installed applications and exposes pinned apps, settings, recent items, and power actions.
- Common shell components start enabled by default and can each be disabled without disabling the whole shell.

## Installation and first run

- The GUI installer detects the distribution, kernel, package manager, desktop session, and available hardware, then selects compatible packages and configuration.
- Users do not need to open a terminal, learn Bash, or identify package names to install, configure, or use LinWin.
- The installer explains the shell-session change, keeps the previous desktop as a recovery option, and provides rollback and uninstall paths.

### File operations and drag-and-drop

- Implemented preview: browse local folders, open files with the host default app, create folders, rename with the file extension left unselected, copy/cut/paste, create a symbolic-link shortcut, and move items to the system Trash.
- Implemented preview: standard keyboard shortcuts, multi-selection, context-menu actions, and drag-and-drop through Qt's file model.
- Remaining: progress for large operations, clearer conflict choices, undo where safe, and broader X11/Wayland and file-manager interoperability checks.
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

- Implemented preview: searchable and sortable process list with per-process CPU share, resident memory, PID, user, and state, refreshed every two seconds.
- Implemented preview: confirmed normal close and force stop for processes owned by the current user, with process-identity checks and no elevation.
- Implemented preview: user-triggered local JSON export with an explicit warning about command lines and file paths.
- Implemented preview: read-only process details via a Details action or row double-click, including parent, executable, working directory, and command line where `/proc` permissions allow.
- Remaining: recognizable application grouping, window-aware app close, history graphs, services/startup views, and narrowly authorized system-process controls.
- Present applications and processes with CPU and memory use, sort/search, and a clear close-app action.
- Make force-quit available for unresponsive apps, with an explicit confirmation when data loss is possible.
- Put services, startup items, and advanced process controls behind an advanced view and the host's authorization mechanisms.

### System configuration

- Provide one searchable entry point for LinWin settings and links into native display, sound, network, printer, account, power, update, privacy, and accessibility panels.
- Prefer invoking the desktop's existing settings module over writing configuration files directly.
- Privileged changes are narrow, authenticated, and reported back to the user.

## Initial delivery order

1. Establish desktop, kernel, hardware, and package compatibility detection.
2. Build the login-session shell core, recovery path, and component settings.
3. Implement the GUI installer with host-specific package selection and rollback.
4. Integrate the familiar taskbar, Start-style launcher, Alt+Tab switching, and file workflows.
5. Add system controls and optional clipboard, Bluetooth, accessibility, and task-manager components.
6. Verify support across targeted distributions, desktops, kernel versions, and X11/Wayland sessions.

The order may change as maintainers learn which integration APIs are reliable on the target desktops.
