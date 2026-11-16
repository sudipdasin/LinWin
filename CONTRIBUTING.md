# Contributing to LinWin

Contributions are welcome. LinWin is intended to work across Linux distributions and desktop environments, so changes should state which environments they affect.

## Before opening a change

1. Check the roadmap and existing issues to avoid duplicate work.
2. For a feature proposal, describe the user problem, expected behaviour, affected desktop environments, and any required system service.
3. Keep changes focused. Avoid adding a dependency for a feature that can use an existing system interface.
4. Never include personal paths, credentials, hostnames, usernames, or device identifiers in code, logs, screenshots, or examples.

## Code conventions

- Use C++20 and Qt 6 for the shared application layer.
- Keep desktop-specific code behind a named adapter.
- Use RAII, value types, and clear ownership; avoid raw owning pointers.
- Do not run the UI as root or use broad privilege escalation.
- Report unavailable system services with actionable messages.
- Add or update documentation for user-visible behaviour and platform support.

## Build and review

Build with the commands in the README. Include the exact distribution, desktop environment, session type (Wayland/X11), and manual verification steps in the pull request description. Do not claim an environment is supported unless it has been checked.

By submitting a contribution, you agree that it may be distributed under LinWin's GPL-3.0-or-later license.
