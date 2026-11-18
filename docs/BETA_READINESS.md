# LinWin public beta readiness

Last reviewed: 2026-11-18

## Release state

LinWin's next preview adds an installable X11 shell session and GUI setup/uninstall flow. The current adapter starts the LinWin shell with XFCE's window manager, settings daemon, and desktop process. It is an experimental session integration, not a release candidate for replacing a daily-use desktop. The GUI registers/removes the selectable session and can uninstall the LinWin package, but does not yet install missing XFCE dependencies or offer per-component toggles.

The previous v0.3.0 beta's C++ build, UI tests, and native package install/removal passed in Ubuntu 24.04, Debian 13, Fedora, and Arch x86_64 containers. The new session helper and wrapper still require CI validation and real desktop testing. Container checks do not prove login manager behavior on a real Mint/XFCE or any other desktop.

## Current test scope

| Area | Current evidence | Public beta status |
| --- | --- | --- |
| Compile | Four-distro container matrix | Automated; desktop runtime not yet verified |
| UI tests | Task manager, clipboard, settings search, and setup-wizard review | Automated subset only |
| Session wrapper | Starts the XFCE session processes and LinWin full-screen entry point; rejects Wayland | Smoke test only; not yet exercised in a display manager on hardware |
| Install package | Native package install/remove and installed session helper files | Automated in distro containers after this change passes |
| Start menu and files | Implemented previews | Manual desktop coverage still needed |
| Task manager | /proc process view and same-user actions | Not equivalent to full Windows Task Manager; services/startup management and broad hardware history are incomplete |
| Clipboard | Opt-in local text history; storage is unencrypted | Privacy limitation; capture defaults off |
| Window switching | X11 adapter uses optional wmctrl; Wayland uses host switcher | No compositor-independent adapter |
| Settings and devices | Opens available host tools and some direct interfaces | Depends on distro packages, services, hardware, and desktop session |
| GUI setup/uninstall | Registers the X11 session through a narrow pkexec helper; uninstalls via APT/DNF/pacman | First implementation; real polkit and package-manager interaction needs VM validation |
| Shell session | XFCE X11 adapter with LinWin full-screen panel, launcher, and sign-out | Experimental; no Wayland, non-XFCE WM, or desktop-manager matrix yet |
| Upgrade / rollback / recovery | Existing session kept; previous session remains selectable; failed uninstall restores session entry | Upgrade/failed-start recovery and safe rollback still require testing |

## Required before a broader shell-replacement beta

- [ ] Add dependency installation and per-component selection; current setup requires XFCE tools already installed.
- [ ] Validate login, lock, logout, sleep/wake, multi-monitor, scaling, keyboard navigation, and accessibility on a named support matrix.
- [ ] Test on clean virtual machines and representative real hardware across X11 and Wayland; publish exact versions and results.
- [ ] Exercise package upgrade, failed upgrade recovery, rollback, and GUI uninstall with each distribution's package manager on real desktop installations.
- [ ] Confirm that logs, screenshots, package metadata, and source contain no unintended host paths, user names, tokens, serials, or other private data.
- [ ] Document known issues, minimum requirements, dependency behavior, privacy implications, and a beginner-friendly recovery path.
- [ ] Review task-manager actions, Bluetooth permissions, clipboard persistence, and all system integration boundaries.
- [ ] Create signed or otherwise verifiable release artifacts and publish checksums, source commit, and reproducible build instructions.

## Safe evaluation

Install this beta only as a normal app and test it in a virtual machine or test account. Do not select LinWin as the only login shell, remove the existing desktop, or depend on it for recovery or system administration. Package manager installation/removal is tested in CI containers; real desktop integration remains unverified.

Report bugs with LinWin commit, distribution/version, desktop/session (X11 or Wayland), hardware model where relevant, and reproduction steps. Remove personal names, paths, hostnames, network identifiers, and serial numbers from diagnostic output before sharing them.
