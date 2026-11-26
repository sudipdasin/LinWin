# Local XFCE candidate build

This isolated development branch adds a dedicated LinWin Updates category, separate from the host software manager. It opens the installed /usr/bin/linwin-update companion when present and otherwise indicates the companion is unavailable. No elevated actions are performed by the shell itself.

On Linux Mint 22.x or Ubuntu 24.04 amd64, install build dependencies through your package manager: build-essential, cmake, pkg-config, qt6-base-dev, libglib2.0-dev, dpkg-dev, file, git, xvfb, xauth. Run `sh scripts/build-xfce-deb.sh` from the repository. It compiles in a temporary directory, runs CTest and an Xvfb startup smoke test, and creates a versioned local candidate DEB plus SHA256 in dist/ only after the gates pass. The script does not install dependencies, upload artifacts, publish releases, or invoke either prohibited CI service. Real XFCE login, upgrade, recovery, and hardware tests are still required before merging.

The local candidate is based on beta.8 and sorts between beta.8 and beta.9. Do not publish the candidate as a stable release.
