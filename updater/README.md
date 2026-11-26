# LinWin Update Center 1.0.3

Separate experimental companion package for LinWin, not a replacement for the native LinWin desktop. Source: Python 3 standard library; Zenity graphical dialogs; native dpkg and APT. Target: Linux Mint 22.x XFCE on amd64 (Ubuntu 24.04 base), Ubuntu 24.04 amd64, Debian 13 amd64. Other platforms are refused. The updater package itself contains architecture-independent Python source.

## Install and use

Open the DEB in your graphical package installer, then launch LinWin Update Center in the Applications menu. Choose preview (including betas) or stable. If a compatible release is available, approve download, review verification and approve installation with the system authentication dialog. Close LinWin before authorizing an upgrade. If LinWin is not installed, the updater offers the compatible package. An equal or older package is never installed.

XFCE checks once per day on login (no background downloads or installation). Notifications report a compatible candidate, not a proven upgrade. Disable the autostart entry in XFCE Session and Startup if desired. The last chosen channel is cached for login checks. No persistent service or elevated daemon.

## Trust model and boundaries

The updater verifies HTTPS, the exact release asset URL, expected SHA256, package name, architecture and Debian package version. If a trusted key is provisioned independently in `/usr/share/keyrings/linwin-release.gpg`, unsigned releases are rejected; a detached SHA256SUMS.sig or SHA256SUMS.asc signature is verified with gpgv. No trust key is bundled. Existing unsigned releases can only be installed after an explicit warning and approval; SHA256 alone is not an independent publisher signature. No unattended installation or automatic rollback is implemented. APT refuses removals and disallowed downgrades; system authentication remains necessary. Installation may fail because of dependencies, network, or missing desktop polkit agent. The updater itself is not automatically updated by this version.

## Validation

Run `python3 test_updater.py`. Tests use fixture releases and real dpkg metadata for synthetic packages. Network, graphical desktop and actual privileged installation on Mint are not covered by these tests. Build using `sh packaging/build.sh` from the source directory. No GitHub Actions or CircleCI required.

## 1.0.2 changes
Strict tag and release-asset metadata policy, signed-release selection when a key is installed, beta tag/package version binding, no notification for an equal or older known beta, and retry after failed background checks. The CPack beta.N package convention is upstream version-N. Stable migration requires a separately designed Debian versioning policy. Run both test_updater.py and test_release_policy.py.

Version 1.0.2 serializes concurrent checks and shows an APT simulation before asking for privileged installation. A failed plan or removal proposal blocks the update. The simulation is advisory: APT re-resolves dependencies at installation time.

Version 1.0.3 refuses plans too large to review and repeats the APT simulation after approval. If the plan changes, it refuses installation. APT still re-resolves dependencies under authorization; this is not a transactional guarantee.
