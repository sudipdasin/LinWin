# LinWin graphical installer specification

The installer is part of the shell product. It should make system changes understandable, reversible, and consistent with the host distribution.

## Shipped preview behavior

The current setup wizard detects distribution, desktop, session, kernel, `pkexec`, and the required XFCE executables. If XFCE components are missing on Debian/Ubuntu/Mint, Fedora/RHEL, or Arch-family systems, it offers a separate step that shows the base packages before the user confirms a polkit authorization prompt. The helper uses a fixed argument list with the native APT, DNF, or pacman executable; it does not invoke a shell, add repositories, request kernel or driver packages, or silently start installation. The package manager may install normal dependencies. Other distributions are directed to their graphical software manager. Once tools are available, the user confirms the session change and the helper writes only `/usr/share/xsessions/linwin.desktop`. The existing desktop and its session definition are left untouched. The first adapter is X11 only and does not configure Wayland or non-XFCE window managers.

The uninstall wizard calls the same allowlisted helper. It removes only the verified LinWin session entry, then invokes the detected APT, DNF, or pacman package manager to remove the `linwin` package. If package removal fails, the session entry is restored. The user can choose **Sign out** in the shell and return to the display manager, then select the previous desktop. Container CI and wrapper tests do not replace real display-manager testing.

## Wizard flow

1. **Welcome:** explain that LinWin replaces the desktop-shell layer while keeping the current Linux distribution, kernel, drivers, and applications.
2. **System check:** detect distribution/version, desktop, X11/Wayland session, hardware capabilities, available storage, and dependencies. Detection is read-only and reports what is supported.
3. **Session choice:** explain the login-session change, preserve the current desktop as a fallback, and let the user choose LinWin or keep the existing shell.
4. **Components:** show the standard components enabled by default, with an individual on/off control for each. Explain disabled, unavailable, and dependency-blocked components. Do not silently install a substitute service.
5. **Permissions and changes:** list packages, session files, permissions, services, and storage changes before applying anything. Reuse the distribution's package manager and host authorization mechanism.
6. **Review:** summarize the selected shell session and components. Make it clear that the user can change these later in LinWin settings.
7. **Install:** show progress and actionable errors. Record enough state to safely roll back a failed operation.
8. **Finish:** verify the installation, offer to start or select the LinWin session, and show how to return to the previous desktop.

## Requirements

- Use native packages or a clearly documented package format for each supported distribution; do not replace the host package manager.
- Do not download, replace, or modify the kernel or hardware drivers as a normal installation step.
- Never perform privileged changes before the review/confirmation step.
- Apply reversible changes transactionally where possible; provide rollback, repair, and complete uninstall paths.
- Keep installation choices available after setup. Components are enabled by default in the standard profile, and every component can be individually disabled.
- Detect incompatible sessions before installation and explain the consequence in plain language.
- Support keyboard navigation, screen readers, text scaling, high contrast, and narrow windows throughout setup.
- Save diagnostics without personal paths, user names, host names, or hardware serial numbers unless the user explicitly chooses to include them.

The remaining requirements below describe work beyond this initial, experimental setup flow, including optional component dependencies, component selection, transaction rollback for package installs, and other display-server adapters.
