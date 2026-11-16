# Changelog

All notable LinWin changes will be recorded here. This project is in early development and has not made a stable release.

## Unreleased

- Added an initial file manager page with folder navigation, open, new folder, extension-aware rename selection, cut/copy/paste, symbolic-link shortcuts, system Trash, keyboard shortcuts, multi-select, context menus, and drag-and-drop.
- Added Files to the in-window shell panel and updated project status, usage guidance, and roadmap.
- Replaced the tab strip with a responsive shell workspace, persistent in-window bottom panel, Home/Clipboard navigation, live date/clock, and a floating searchable Start menu.
- Added keyboard-friendly Start search focus on opening and a clear Escape dismissal path when the main window has focus.
- Added a searchable Start-style application launcher that discovers registered apps, launches them through GIO, and stores pinned favorites.
- Defined the core audience as Windows switchers and Linux beginners who should be able to install and use LinWin without terminal or Bash knowledge.
- Specified a unified, familiar shell experience with automatic distro, kernel, hardware, and package detection in the GUI installer.
- Clarified the modular component defaults, performance direction, system-access boundary, recovery path, and graphical setup flow.
- Corrected prototype UI and package descriptions so they do not imply the complete shell is already implemented.

## 0.2.0 - Clipboard history preview

- Added an opt-in clipboard tab for recent text, with search, copy, remove, and clear actions.
- Stored up to 50 clips in local Qt application settings; capture is disabled by default and documented as unencrypted.
- Replaced the non-interactive sidebar labels with compact Overview and Clipboard tabs for narrow-window navigation.
- Added clear project documentation about capture lifetime, storage, and current limitations.

## 0.1.0 - Initial development scaffold

- Added a C++20 and Qt 6 desktop application.
- Added detection and display of distribution, desktop session, session type, and kernel.
- Added a responsive introductory dashboard and explicit development-status messaging.
- Added contributor, security, license, and roadmap documents.
- Added a container-based build workflow for Ubuntu, Debian, Fedora, and Arch Linux.
