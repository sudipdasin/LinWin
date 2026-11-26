# Local release validation

Before offering a LinWin release, check its published filenames and direct
download URLs, then inspect both Debian package variants. This check is
read-only and does not run or change GitHub Actions or CircleCI.

From the repository root, run:

```sh
python3 -m unittest tests/test_release_asset_validation.py -v
python3 scripts/validate_release_assets.py \
  --tag v1.0.0-beta.8 \
  --live \
  --ubuntu-deb build-ubuntu/linwin-1.0.0-beta.8-ubuntu-24.04.deb \
  --debian-deb build-debian/linwin-1.0.0-beta.8-debian-13.deb
```

`--live` performs a public metadata GET only. For an offline audit, save the
release API JSON and pass it with `--release-json saved-release.json` instead.
The check fails if any expected release asset is missing, a direct download URL
does not match its release tag and filename, or the package dependencies do not
match the selected target. Ubuntu-based Linux Mint must use the Ubuntu 24.04
package; Debian 13 has a separate Qt and C++ runtime dependency baseline.
