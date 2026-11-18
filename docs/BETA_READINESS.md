# LinWin public beta readiness

Last reviewed: 2026-11-18

## Release state

LinWin is a **pre-beta engineering preview**, not a release candidate for replacing a daily-use desktop. The current Qt application runs inside its own window. It does not start as a login session, install a desktop shell, or include the promised graphical setup and recovery wizard.

CI currently verifies a C++ build, automated UI tests, and a staged file install inside Ubuntu 24.04, Debian 13, Fedora, and Arch Linux containers. Pull requests also produce short-lived TGZ preview artifacts and SHA-256 checksum files. A successful container build does not prove that the shell works on a real Mint, XFCE, Cinnamon, MATE, Plasma, X11, or Wayland desktop.

## Current test scope

| Area | Current evidence | Public beta status |
| --- | --- | --- |
| Compile | Four-distro container matrix | Automated; desktop runtime not yet verified |
| UI tests | Task manager, clipboard, and settings search tests | Automated subset only |
| Install tree | CI checks the executable and desktop entry after staging install | Automated; no package-manager install/uninstall test |
| Start menu and files | Implemented previews | Manual desktop coverage still needed |
| Task manager | /proc process view and same-user actions | Not equivalent to full Windows Task Manager; services/startup management and broad hardware history are incomplete |
| Clipboard | Opt-in local text history; storage is unencrypted | Privacy limitation; capture defaults off |
| Window switching | X11 adapter uses optional wmctrl; Wayland uses host switcher | No compositor-independent adapter |
| Settings and devices | Opens available host tools and some direct interfaces | Depends on distro packages, services, hardware, and desktop session |
| Shell session / installer | Not implemented | Release blocker |
| Upgrade / rollback / recovery | Not implemented | Release blocker |

## Required before announcing a public beta

- [ ] Implement the graphical installer, component selection, session registration, safe return to the previous desktop, and complete uninstall.
- [ ] Validate login, lock, logout, sleep/wake, multi-monitor, scaling, keyboard navigation, and accessibility on a named support matrix.
- [ ] Test on clean virtual machines and representative real hardware across X11 and Wayland; publish exact versions and results.
- [ ] Exercise package install, upgrade, failed upgrade recovery, rollback, and uninstall with the distribution's package manager.
- [ ] Confirm that logs, screenshots, package metadata, and source contain no unintended host paths, user names, tokens, serials, or other private data.
- [ ] Document known issues, minimum requirements, dependency behavior, privacy implications, and a beginner-friendly recovery path.
- [ ] Review task-manager actions, Bluetooth permissions, clipboard persistence, and all system integration boundaries.
- [ ] Create signed or otherwise verifiable release artifacts and publish checksums, source commit, and reproducible build instructions.

## Safe evaluation

Until the release blockers above are closed, run LinWin as a normal application in a virtual machine or test account. Do not select it as the only login shell, remove the existing desktop, or depend on it for recovery or system administration. CI preview archives are temporary developer artifacts, not supported installer packages.

Report bugs with LinWin commit, distribution/version, desktop/session (X11 or Wayland), hardware model where relevant, and reproduction steps. Remove personal names, paths, hostnames, network identifiers, and serial numbers from diagnostic output before sharing them.
