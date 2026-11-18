# Security policy

Do not report exploitable vulnerabilities in public issues. Until a private security contact is published, report security concerns through the private vulnerability-reporting feature of the public repository. If that feature is unavailable, contact the project maintainers privately through the repository hosting provider.

Please include the affected version or commit, impact, reproduction steps, and a suggested fix if available. Do not include real credentials, personal files, or unredacted system logs.

LinWin is early development and is not a security boundary. Its privileged helper is limited to registering/removing the LinWin X11 session file and requesting removal of the named LinWin package through APT, DNF, or pacman after an explicit GUI confirmation. The helper accepts no arbitrary commands or package names. The shell and GUI must run as the normal user; do not run the desktop shell as root.
