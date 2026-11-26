#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Check and install checksum-verified LinWin releases over HTTPS for supported .deb systems."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any
from urllib.error import HTTPError, URLError
from urllib.parse import urlparse
from urllib.request import HTTPRedirectHandler, Request, build_opener

REPOSITORY = "sudipdasin/LinWin"
API_URL = f"https://api.github.com/repos/{REPOSITORY}/releases?per_page=100"
DOWNLOAD_ROOT = f"https://github.com/{REPOSITORY}/releases/download"
USER_AGENT = "LinWin-In-App-Updater"
MAX_RELEASE_BYTES = 100 * 1024 * 1024


class HttpsRedirectHandler(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        parsed = urlparse(newurl)
        if parsed.scheme != "https" or parsed.hostname not in {
            "api.github.com", "github.com", "release-assets.githubusercontent.com", "objects.githubusercontent.com"
        }:
            raise ValueError("Refusing an insecure update redirect")
        return super().redirect_request(req, fp, code, msg, headers, newurl)


def fetch(url: str, *, limit: int = MAX_RELEASE_BYTES) -> bytes:
    if urlparse(url).scheme != "https":
        raise ValueError("Update URLs must use HTTPS")
    request = Request(url, headers={"User-Agent": USER_AGENT, "Accept": "application/vnd.github+json"})
    opener = build_opener(HttpsRedirectHandler())
    try:
        with opener.open(request, timeout=30) as response:
            final_url = response.geturl()
            if urlparse(final_url).scheme != "https":
                raise ValueError("Refusing an insecure update download")
            length = response.headers.get("Content-Length")
            if length and int(length) > limit:
                raise ValueError("Update download exceeds the 100 MiB safety limit")
            data = response.read(limit + 1)
            if len(data) > limit:
                raise ValueError("Update download exceeds the 100 MiB safety limit")
            return data
    except (HTTPError, URLError, TimeoutError) as exc:
        raise ValueError(f"Update server could not be reached: {exc}") from exc


def read_os_release(path: Path = Path("/etc/os-release")) -> dict[str, str]:
    values: dict[str, str] = {}
    if not path.is_file():
        return values
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key] = value.strip().strip("\"'")
    return values


def package_target(os_release: dict[str, str]) -> str:
    distro = os_release.get("ID", "").lower()
    version = os_release.get("VERSION_ID", "")
    if distro == "linuxmint" and re.match(r"^22(?:\.|$)", version):
        return "ubuntu"
    if distro == "ubuntu" and version.startswith("24.04"):
        return "ubuntu"
    if distro == "debian" and version.split(".", 1)[0] == "13":
        return "debian"
    raise ValueError("In-app updates currently support Linux Mint 22, Ubuntu 24.04, and Debian 13 (amd64) only")


def tag_key(tag: str) -> tuple[int, int, int, int, int] | None:
    match = re.fullmatch(r"v?(\d+)\.(\d+)\.(\d+)(?:-beta\.(\d+))?", tag)
    if not match:
        return None
    major, minor, patch = (int(match.group(i)) for i in (1, 2, 3))
    beta = match.group(4)
    return (major, minor, patch, 0 if beta else 1, int(beta or 0))


def package_version_for_tag(tag: str) -> str:
    match = re.fullmatch(r"v?(\d+\.\d+\.\d+)-beta\.(\d+)", tag)
    if not match:
        raise ValueError(f"No Debian package version mapping exists for release {tag}")
    return f"{match.group(1)}-{match.group(2)}"


def expected_asset(tag: str, target: str) -> str:
    if target not in {"ubuntu", "debian"}:
        raise ValueError(f"Unsupported package target: {target}")
    suffix = "ubuntu-24.04" if target == "ubuntu" else "debian-13"
    return f"linwin-{tag.removeprefix('v')}-{suffix}.deb"


def choose_release(releases: list[dict[str, Any]], target: str) -> dict[str, Any]:
    # Debian revision mapping is explicit for beta.N releases. Stable releases
    # require a matching package-version policy before the updater can accept them.
    candidates = [release for release in releases if not release.get("draft")
                  and re.fullmatch(r"v?\d+\.\d+\.\d+-beta\.\d+", str(release.get("tag_name", "")))]
    candidates.sort(key=lambda release: tag_key(str(release["tag_name"])), reverse=True)
    for release in candidates:
        tag = str(release["tag_name"])
        asset_name = expected_asset(tag, target)
        assets = {asset.get("name"): asset for asset in release.get("assets", []) if isinstance(asset, dict)}
        asset = assets.get(asset_name)
        checksums = assets.get("SHA256SUMS")
        if not asset or not checksums:
            continue
        expected_url = f"{DOWNLOAD_ROOT}/{tag}/{asset_name}"
        if asset.get("browser_download_url") != expected_url:
            continue
        if checksums.get("browser_download_url") != f"{DOWNLOAD_ROOT}/{tag}/SHA256SUMS":
            continue
        return {"tag": tag, "version": package_version_for_tag(tag), "asset": asset_name,
                "asset_url": expected_url, "checksums_url": f"{DOWNLOAD_ROOT}/{tag}/SHA256SUMS"}
    raise ValueError(f"No complete published LinWin release supports the {target} package target")


def installed_package_version() -> str:
    result = subprocess.run(["/usr/bin/dpkg-query", "-W", "-f=${Version}", "linwin"],
                            capture_output=True, text=True, check=False)
    if result.returncode != 0 or not result.stdout.strip():
        raise ValueError("LinWin is not installed as a Debian package, so in-app upgrade is unavailable")
    return result.stdout.strip()


def is_newer(candidate: str, installed: str) -> bool:
    result = subprocess.run(["/usr/bin/dpkg", "--compare-versions", installed, "lt", candidate], check=False)
    return result.returncode == 0


def version_tuple(version: str) -> tuple[int, ...]:
    try:
        return tuple(int(part) for part in version.split("."))
    except ValueError as exc:
        raise ValueError(f"Unexpected Qt dependency version: {version}") from exc


def validate_target_dependencies(target: str, depends: str) -> None:
    normalized = " ".join(depends.split())
    core = re.search(r"libqt6core6t64\s*\(>=\s*([0-9.]+)\)", normalized)
    if not core:
        raise ValueError("Package is missing the expected Qt 6 Core dependency")
    qt_version = version_tuple(core.group(1))
    if target == "ubuntu":
        if not (version_tuple("6.4.0") <= qt_version <= version_tuple("6.4.2")):
            raise ValueError(f"Ubuntu/Mint package requires Qt {core.group(1)}; supported range is 6.4.0 through 6.4.2")
        if not re.search(r"libqt6gui6t64(?:\s|,|\()", normalized) or not re.search(r"libqt6widgets6t64(?:\s|,|\()", normalized):
            raise ValueError("Ubuntu/Mint package must use the t64 Qt GUI and Widgets libraries")
        stdcxx = re.search(r"libstdc\+\+6\s*\(>=\s*(\d+)", normalized)
        if stdcxx and int(stdcxx.group(1)) >= 14:
            raise ValueError("Ubuntu/Mint package must not require Debian 13's libstdc++ 14")
        return
    if target == "debian":
        if qt_version < version_tuple("6.8.2"):
            raise ValueError(f"Debian 13 package requires Qt {core.group(1)}; expected at least 6.8.2")
        if re.search(r"libqt6gui6t64(?:\s|,|\()|libqt6widgets6t64(?:\s|,|\()", normalized):
            raise ValueError("Debian 13 package must use the non-t64 Qt GUI and Widgets libraries")
        if not re.search(r"libqt6gui6(?:\s|,|\()", normalized) or not re.search(r"libqt6widgets6(?:\s|,|\()", normalized):
            raise ValueError("Debian 13 package must depend on the non-t64 Qt GUI and Widgets libraries")
        stdcxx = re.search(r"libstdc\+\+6\s*\(>=\s*(\d+)", normalized)
        if not stdcxx or int(stdcxx.group(1)) < 14:
            raise ValueError("Debian 13 package must require libstdc++6 >= 14")
        return
    raise ValueError(f"Unsupported package target: {target}")


def release_info(target: str, installed: str) -> dict[str, str]:
    releases = json.loads(fetch(API_URL))
    if not isinstance(releases, list):
        raise ValueError("Update server returned an unexpected release list")
    latest = choose_release(releases, target)
    if is_newer(latest["version"], installed):
        return {"status": "update", "current": installed, **latest}
    return {"status": "current", "current": installed, "latest": latest["version"], "tag": latest["tag"]}


def expected_checksum(checksum_text: str, asset_name: str) -> str:
    matches = re.findall(rf"^([0-9a-fA-F]{{64}})\s+\*?{re.escape(asset_name)}\s*$", checksum_text, re.MULTILINE)
    if len(matches) != 1:
        raise ValueError(f"SHA256SUMS must contain exactly one entry for {asset_name}")
    return matches[0].lower()


def verify_checksum(package_data: bytes, checksum_text: str, asset_name: str) -> None:
    expected = expected_checksum(checksum_text, asset_name)
    actual = hashlib.sha256(package_data).hexdigest()
    if actual != expected:
        raise ValueError("Downloaded package checksum does not match this release's SHA256SUMS")


def validate_package(package: Path, target: str, expected_version: str) -> None:
    fields = subprocess.run(["/usr/bin/dpkg-deb", "-f", str(package), "Package", "Version", "Architecture", "Depends"],
                            capture_output=True, text=True, check=False)
    if fields.returncode != 0:
        raise ValueError("Downloaded file is not a readable Debian package")
    metadata: dict[str, str] = {}
    active_field = ""
    for line in fields.stdout.splitlines():
        if line[:1].isspace() and active_field:
            metadata[active_field] = f"{metadata[active_field]} {line.strip()}".strip()
        elif ":" in line:
            active_field, value = line.split(":", 1)
            metadata[active_field.strip()] = value.strip()
    if [metadata.get(name, "") for name in ("Package", "Version", "Architecture")] != ["linwin", expected_version, "amd64"]:
        raise ValueError("Downloaded package identity, version, or architecture did not match the selected release")
    validate_target_dependencies(target, metadata.get("Depends", ""))
    if not is_newer(expected_version, installed_package_version()):
        raise ValueError("The downloaded package is not newer than the installed LinWin version")


def install_update(target: str, tag: str) -> int:
    releases = json.loads(fetch(API_URL))
    if not isinstance(releases, list):
        raise ValueError("Update server returned an unexpected release list")
    selected = choose_release(releases, target)
    if selected["tag"] != tag:
        raise ValueError("The selected release is no longer the newest supported release; check again")
    checksums = fetch(selected["checksums_url"], limit=1024 * 1024).decode("utf-8", errors="strict")
    expected_checksum(checksums, selected["asset"])

    cache_base = os.environ.get("XDG_CACHE_HOME") or str(Path.home() / ".cache")
    cache_root = Path(cache_base) / "linwin" / "updates"
    cache_root.mkdir(mode=0o700, parents=True, exist_ok=True)
    if cache_root.is_symlink() or cache_root.stat().st_uid != os.geteuid():
        raise ValueError("The LinWin update cache directory is not owned by this user")
    os.chmod(cache_root, 0o700)
    with tempfile.TemporaryDirectory(prefix="verified-", dir=cache_root) as staging:
        package = Path(staging) / selected["asset"]
        data = fetch(selected["asset_url"])
        verify_checksum(data, checksums, selected["asset"])
        with package.open("xb") as output:
            output.write(data)
        os.chmod(package, 0o600)
        validate_package(package, target, selected["version"])

        apt = "/usr/bin/apt-get"
        pkexec = "/usr/bin/pkexec"
        if not os.access(apt, os.X_OK):
            raise ValueError("apt-get was not found; install this update through the system package manager")
        command = [apt, "install", "--yes", str(package)]
        if os.geteuid() != 0:
            if not os.access(pkexec, os.X_OK):
                raise ValueError("pkexec is unavailable; install this update through the system package manager")
            command.insert(0, pkexec)
        print(f"Installing LinWin {selected['version']}. Approve the system authorization prompt to continue.", flush=True)
        result = subprocess.run(command, check=False)
        return result.returncode


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("check", "install"))
    parser.add_argument("--tag", help="Release tag selected by the GUI for installation")
    args = parser.parse_args(argv)
    try:
        architecture = subprocess.run(["/usr/bin/dpkg", "--print-architecture"],
                                      capture_output=True, text=True, check=False)
        if architecture.returncode != 0 or architecture.stdout.strip() != "amd64":
            raise ValueError("In-app updates currently support amd64 packages only")
        target = package_target(read_os_release())
        if args.command == "check":
            print(json.dumps(release_info(target, installed_package_version())))
            return 0
        if not args.tag:
            raise ValueError("The install command requires --tag")
        return install_update(target, args.tag)
    except (OSError, UnicodeError, json.JSONDecodeError, ValueError, subprocess.SubprocessError) as exc:
        if args.command == "check":
            print(json.dumps({"status": "error", "message": str(exc)}))
            return 0
        print(f"Update failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
