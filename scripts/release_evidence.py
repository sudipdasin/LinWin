#!/usr/bin/env python3
"""Publish measured release evidence only after the native build gates pass."""
import hashlib
import json
import re
import struct
import subprocess
import sys
from pathlib import Path
from xml.etree import ElementTree

EXPECTED = {'theme-catalog', 'snap-geometry', 'workspace-profiles', 'recent-documents', 'background-preferences', 'shell-layout-policy',
            'neon-landscape', 'terrain-texture', 'pin-suggestions', 'weather-forecast',
            'desktop-status', 'system-metrics', 'task-manager-tabs', 'clipboard-center',
            'settings-hub-search', 'setup-wizard-review', 'file-manager-quick-access',
            'xfce-dependency-plan', 'responsive-taskbar', 'x11-session-wrapper'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def collect(root, filename, version):
    if not re.fullmatch(r'linwin-[0-9][A-Za-z0-9.+~_-]*\.deb', filename):
        raise ValueError('Unexpected package filename')
    pkg = root / filename
    if not pkg.is_file() or pkg.is_symlink():
        raise ValueError('Missing versioned package')
    for field, expected in [('Package', 'linwin'), ('Architecture', 'amd64'), ('Version', version)]:
        actual = subprocess.check_output(['dpkg-deb', '-f', str(pkg), field], text=True).strip()
        if actual != expected:
            raise ValueError(f'Package {field} mismatch')
    sha = digest(pkg)
    if (root / 'SHA256SUMS').read_text().strip() != f'{sha}  {filename}':
        raise ValueError('Package checksum mismatch')
    cases = ElementTree.parse(root / 'ctest-results.xml').findall('.//testcase')
    names = [c.attrib.get('name') for c in cases]
    if len(names) != len(EXPECTED) or set(names) != EXPECTED:
        raise ValueError('Native suite inventory mismatch')
    if any(c.find(tag) is not None for c in cases for tag in ('failure', 'error', 'skipped')):
        raise ValueError('Native test did not pass')
    for log, count in [('updater-tests.txt', 45), ('download-tests.txt', 4)]:
        content = (root / log).read_text()
        if not re.search(rf'^Ran {count} tests? in ', content, re.MULTILINE) or not content.rstrip().endswith('OK'):
            raise ValueError(f'Test log not successful: {log}')
    captures = []
    for size in ('1672x941', '1920x1080'):
        for mode in ('desktop', 'start'):
            rel = f'resolution-checks/neon-{size}-{mode}.png'
            image = root / rel
            data = image.read_bytes()
            dimensions = tuple(map(int, size.split('x')))
            if data[:8] != b'\x89PNG\r\n\x1a\n' or len(data) < 24 or struct.unpack('>II', data[16:24]) != dimensions:
                raise ValueError(f'Unexpected capture dimensions: {rel}')
            captures.append({'path': rel, 'width': dimensions[0], 'height': dimensions[1], 'sha256': digest(image)})
    return {'package': {'filename': filename, 'version': version, 'architecture': 'amd64',
                         'bytes': pkg.stat().st_size, 'sha256': sha},
            'native_suites_passed': len(cases), 'native_suite_names': sorted(names),
            'updater_tests_passed': 45, 'download_tests_passed': 4,
            'resolution_captures': captures, 'visual_reference_accepted': False,
            'physical_xfce_certified': False, 'environment': 'Ubuntu 24.04 native build and virtual display'}


if __name__ == '__main__':
    root = Path(sys.argv[1])
    result = collect(root, sys.argv[2], sys.argv[3])
    (root / 'release-evidence.json').write_text(json.dumps(result, indent=2) + '\n')
    print(f"PASS: release evidence binds {result['native_suites_passed']} passing native suites and package SHA256")
