#!/usr/bin/env python3
"""Validate LinWin release downloads and Debian package target dependencies.

This is a local, read-only release check. It never runs CI, edits releases, or
uploads artifacts. Supply a saved GitHub release JSON response or use --live.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

REPOSITORY = "sudipdasin/LinWin"
DOWNLOAD_ROOT = f"https://github.com/{REPOSITORY}/releases/download"


def version_tuple(value: str) -> tuple[int, int, int]:
    parts = [int(part) for part in value.split(".")]
    return tuple((parts + [0, 0, 0])[:3])


def expected_assets(tag: str) -> set[str]:
    version = tag.removeprefix("v")
    return {
        f"linwin-{version}-ubuntu-24.04.deb",
        f"linwin-{version}-debian-13.deb",
        f"linwin-{version}-fedora.rpm",
        f"linwin-{version}-arch-x86_64.pkg.tar.zst",
        f"LinWin-{version}-linux-packages.zip",
        "SHA256SUMS",
    }


def parse_release(data: Any, tag: str) -> dict[str, Any]:
    if isinstance(data, list):
        matches = [release for release in data if release.get("tag_name") == tag]
        if not matches:
            raise ValueError(f"Release {tag} is missing from the supplied release list")
        return matches[0]
    if isinstance(data, dict) and data.get("tag_name") == tag:
        return data
    raise ValueError(f"Release JSON does not describe tag {tag}")


def validate_release(release: dict[str, Any], tag: str) -> list[str]:
    errors: list[str] = []
    assets = release.get("assets")
    if not isinstance(assets, list):
        return ["Release JSON has no assets array"]

    by_name: dict[str, dict[str, Any]] = {}
    for asset in assets:
        name = asset.get("name")
        if not isinstance(name, str):
            errors.append("Release contains an asset with no filename")
            continue
        by_name[name] = asset

    expected = expected_assets(tag)
    for missing in sorted(expected - by_name.keys()):
        errors.append(f"Missing release asset: {missing}")

    for name in sorted(expected & by_name.keys()):
        expected_url = f"{DOWNLOAD_ROOT}/{tag}/{name}"
        actual_url = by_name[name].get("browser_download_url")
        if actual_url != expected_url:
            errors.append(
                f"Wrong download URL for {name}: expected {expected_url}, got {actual_url!r}"
            )
    return errors


def read_depends(deb_path: Path) -> str:
    result = subprocess.run(
        ["dpkg-deb", "-f", str(deb_path), "Depends"],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode:
        raise ValueError(result.stderr.strip() or f"Cannot read package metadata: {deb_path}")
    return result.stdout.strip()


def read_package_version(deb_path: Path) -> str:
    result = subprocess.run(
        ["dpkg-deb", "-f", str(deb_path), "Version"],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode:
        raise ValueError(result.stderr.strip() or f"Cannot read package version: {deb_path}")
    return result.stdout.strip()


def expected_deb_version(tag: str) -> str:
    match = re.fullmatch(r"v?(\d+\.\d+\.\d+)-beta\.(\d+)", tag)
    if not match:
        raise ValueError(f"Cannot derive Debian package version from beta tag: {tag}")
    return f"{match.group(1)}-{match.group(2)}"


def validate_deb(deb_path: Path, target: str, depends: str) -> list[str]:
    errors: list[str] = []
    normalized = " ".join(depends.split())
    if target == "ubuntu":
        core = re.search(r"libqt6core6t64\s*\(>=\s*([0-9.]+)\)", normalized)
        if not core:
            errors.append(f"{deb_path}: Ubuntu/Mint package needs libqt6core6t64")
        elif version_tuple(core.group(1)) > (6, 4, 2):
            errors.append(
                f"{deb_path}: Ubuntu/Mint package requires Qt {core.group(1)}; Mint 22.3 / Ubuntu 24.04 provides Qt 6.4.2"
            )
        if "libqt6gui6t64" not in normalized or "libqt6widgets6t64" not in normalized:
            errors.append(f"{deb_path}: Ubuntu/Mint package must depend on the *t64 Qt GUI and Widgets libraries")
        if re.search(r"libstdc\+\+6\s*\(>=\s*14\)", normalized):
            errors.append(f"{deb_path}: Ubuntu/Mint package must not require Debian 13's libstdc++ 14")
    elif target == "debian":
        core = re.search(r"libqt6core6t64\s*\(>=\s*([0-9.]+)\)", normalized)
        if not core:
            errors.append(f"{deb_path}: Debian 13 package needs libqt6core6t64")
        elif version_tuple(core.group(1)) < (6, 8, 2):
            errors.append(
                f"{deb_path}: Debian 13 package requires Qt {core.group(1)}; expected at least 6.8.2"
            )
        if "libqt6gui6t64" in normalized or "libqt6widgets6t64" in normalized:
            errors.append(f"{deb_path}: Debian 13 package must use libqt6gui6 and libqt6widgets6")
        if "libqt6gui6" not in normalized or "libqt6widgets6" not in normalized:
            errors.append(f"{deb_path}: Debian 13 package must depend on libqt6gui6 and libqt6widgets6")
        stdcxx = re.search(r"libstdc\+\+6\s*\(>=\s*([0-9]+)\)", normalized)
        if not stdcxx or int(stdcxx.group(1)) < 14:
            errors.append(f"{deb_path}: Debian 13 package must require libstdc++6 >= 14")
    else:
        errors.append(f"Unsupported Debian package target: {target}")
    return errors


def load_release(path: Path | None, tag: str, live: bool) -> dict[str, Any]:
    if live:
        url = f"https://api.github.com/repos/{REPOSITORY}/releases/tags/{tag}"
        request = Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "LinWin-release-audit"})
        try:
            with urlopen(request, timeout=15) as response:
                data = json.load(response)
        except (HTTPError, URLError, TimeoutError) as exc:
            raise ValueError(f"Could not read release metadata from {url}: {exc}") from exc
    elif path is not None:
        data = json.loads(path.read_text(encoding="utf-8"))
    else:
        raise ValueError("Supply --release-json FILE or --live")
    return parse_release(data, tag)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tag", required=True, help="Release tag, for example v1.0.0-beta.8")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--release-json", type=Path, help="Saved release JSON, for offline validation")
    source.add_argument("--live", action="store_true", help="Read public release metadata (GET only)")
    parser.add_argument("--ubuntu-deb", type=Path, help="Check .deb dependencies for Ubuntu 24.04 / Mint")
    parser.add_argument("--debian-deb", type=Path, help="Check .deb dependencies for Debian 13")
    args = parser.parse_args(argv)

    try:
        release = load_release(args.release_json, args.tag, args.live)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    errors = validate_release(release, args.tag)
    for deb_path, target in ((args.ubuntu_deb, "ubuntu"), (args.debian_deb, "debian")):
        if deb_path is None:
            continue
        if not deb_path.is_file():
            errors.append(f"Package file does not exist: {deb_path}")
            continue
        try:
            package_version = read_package_version(deb_path)
            expected_version = expected_deb_version(args.tag)
            if package_version != expected_version:
                errors.append(
                    f"{deb_path}: package version is {package_version}; release {args.tag} expects {expected_version}"
                )
            depends = read_depends(deb_path)
        except (OSError, ValueError) as exc:
            errors.append(str(exc))
            continue
        errors.extend(validate_deb(deb_path, target, depends))

    if errors:
        for error in errors:
            print(f"FAIL: {error}")
        return 1
    print(f"PASS: {args.tag} has all expected release assets and valid download URLs")
    if args.ubuntu_deb:
        print(f"PASS: Ubuntu/Mint dependencies are compatible in {args.ubuntu_deb}")
    if args.debian_deb:
        print(f"PASS: Debian 13 dependencies are compatible in {args.debian_deb}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
