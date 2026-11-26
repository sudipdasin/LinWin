# LinWin Update Center 1.0.1

Experimental Python and Zenity graphical updater companion. Supports Mint 22.x with Ubuntu noble base, Ubuntu 24.04 and Debian 13 on amd64. Run as a desktop user, not root. Use the Applications menu to choose a channel and review a candidate before installation. XFCE login checks once daily; no unattended installations.

Build locally: `sh packaging/build.sh`. Run the release policy tests: `python3 test_release_policy.py`. The complete local test suite is also supplied in the source archive.

A trusted key at /usr/share/keyrings/linwin-release.gpg enables signed-release enforcement. Without a key, unsigned releases need explicit approval and are checked only against HTTPS and SHA256. A signature asset without a trusted key is rejected. The beta.N mapping uses upstream version-N; stable migration needs a Debian versioning policy. No automatic rollback or updater self-update is implemented. Real Mint desktop and network installation are not yet verified.
