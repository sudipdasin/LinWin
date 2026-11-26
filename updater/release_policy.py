# SPDX-License-Identifier: GPL-3.0-or-later
"""Fail-closed metadata validation for LinWin release candidates."""
import re

TAG = re.compile(r'v?(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(?:-(alpha|beta|rc)\.([1-9][0-9]*))?\Z')
BASE = 'https://github.com/sudipdasin/LinWin/releases/download/'


def version_key(tag):
    m = TAG.fullmatch(tag) if isinstance(tag, str) and len(tag) <= 80 else None
    if not m:
        return None
    return tuple(map(int, m.group(1, 2, 3))) + ({'alpha': 0, 'beta': 1, 'rc': 2, None: 3}[m[4]], int(m[5] or 0))


def select_release(entries, suffix, channel, require_signature=False):
    if suffix not in ('ubuntu-24.04', 'debian-13') or channel not in ('stable', 'preview'):
        raise ValueError('Unsupported release channel or platform.')
    candidates = []
    seen = set()
    for release in entries:
        if not isinstance(release, dict):
            raise ValueError('Malformed release entry.')
        tag = release.get('tag_name')
        key = version_key(tag)
        if key is None or release.get('draft') is not False or not isinstance(release.get('published_at'), str) or not release['published_at']:
            continue
        if release.get('prerelease') is not (key[3] != 3):
            continue
        if channel == 'stable' and key[3] != 3:
            continue
        if key in seen:
            raise ValueError('Ambiguous release versions.')
        seen.add(key)
        name = f'linwin-{tag.removeprefix("v")}-{suffix}.deb'
        assets = {}
        if not isinstance(release.get('assets'), list):
            continue
        for asset in release['assets']:
            if not isinstance(asset, dict) or not isinstance(asset.get('name'), str):
                raise ValueError('Malformed release asset.')
            n = asset['name']
            if n in assets:
                raise ValueError('Duplicate release asset.')
            assets[n] = asset
        required = [name, 'SHA256SUMS']
        if require_signature:
            signatures = [n for n in ('SHA256SUMS.sig', 'SHA256SUMS.asc') if n in assets]
            if len(signatures) != 1:
                continue
            required += signatures
        if any(n not in assets or assets[n].get('browser_download_url') != BASE + tag + '/' + n for n in required):
            continue
        candidates.append((key, tag, name, assets))
    if not candidates:
        raise ValueError('No compatible release satisfying the trust policy was found.')
    return max(candidates, key=lambda x: x[0])[1:]


def beta_package_version(tag):
    """Current CPack convention: beta.N -> upstream version-N; no other assumptions."""
    key = version_key(tag)
    if key is None or key[3] != 1:
        return None
    return '.'.join(map(str, key[:3])) + '-' + str(key[4])


def verify_release_version(tag, package_version):
    expected = beta_package_version(tag)
    if expected is not None and package_version != expected:
        raise ValueError('Package version does not match the beta release tag.')
    if expected is None:
        key = version_key(tag)
        if key is None or package_version.split('-', 1)[0].split('~', 1)[0] != '.'.join(map(str, key[:3])):
            raise ValueError('Package upstream version does not match the release tag.')
