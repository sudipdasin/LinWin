# Release policy foundation

This branch contains the locally regression-tested pure Python release selection policy for the separately packaged LinWin Update Center 1.0.1. Run `python3 updater/test_release_policy.py` without network or CI. The full companion application and DEB are supplied separately; this branch alone does not build the updater GUI.

The current CPack beta.N mapping is upstream-version-N. A stable migration needs a Debian versioning policy that orders stable above earlier betas. The presence of a signature asset is not proof of authenticity: callers must verify the signature using an independently provisioned trusted key, then verify package digest, name, architecture and version before requesting installation. No unattended installation.
