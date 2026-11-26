# In-app updates

The update center checks once per day by default and can also be run manually
from **Settings → App updates**. It only notifies about an update; it never
installs one without the user's confirmation. Installation runs through
`pkexec` and `apt-get`, so the desktop shows the normal system authorization
prompt and package-manager errors remain visible in LinWin.

The first updater supports amd64 packages on Linux Mint 22.x and Ubuntu 24.04
(the Ubuntu package), and Debian 13 (the Debian package). Other distributions,
architectures, and Wayland-specific session integrations are not supported by
this updater milestone. The updater selects the newest complete `beta.N`
release numerically, so `beta.10` sorts after `beta.9`; it skips drafts and
releases without the correct package and `SHA256SUMS` file.

Before installation it requires the exact expected release download URL,
downloads the matching package and checksum over HTTPS, checks the SHA-256
digest, inspects package name/version/architecture, and compares the Debian
package version with the installed one using `dpkg`. The checksum file is not
currently signed, so this check detects download corruption but does not
independently authenticate the release publisher. Stable tags are intentionally
skipped until their Debian package-version mapping is documented and tested.

Manual checks remain available when automatic checks are disabled. Network
failures do not block LinWin's normal shell features. If the package manager is
busy, authorization is cancelled, or installation fails, LinWin reports the
failure and leaves the currently installed package in place.
