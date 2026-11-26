import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
from xml.etree import ElementTree as ET
import struct
from release_evidence import collect, digest, EXPECTED


class ReleaseEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.name = 'linwin-1.0.1-preview.7-ubuntu-24.04-amd64.deb'
        self.version = '1.0.1~preview.7-1'
        pkg = self.root / self.name
        pkg.write_bytes(b'fixture package bytes')
        (self.root / 'SHA256SUMS').write_text(f'{digest(pkg)}  {self.name}\n')
        self.xml = ET.Element('testsuite')
        for name in sorted(EXPECTED):
            ET.SubElement(self.xml, 'testcase', name=name)
        self.write_xml()
        for log, count in [('updater-tests.txt', 45), ('download-tests.txt', 4)]:
            (self.root / log).write_text(f'Ran {count} tests in 0.1s\n\nOK\n')
        (self.root / 'resolution-checks').mkdir()
        for w, h in [(1672, 941), (1920, 1080)]:
            for mode in ('desktop', 'start'):
                (self.root / f'resolution-checks/neon-{w}x{h}-{mode}.png').write_bytes(b'\x89PNG\r\n\x1a\n' + b'\x00' * 8 + struct.pack('>II', w, h))
        self.metadata = patch('release_evidence.subprocess.check_output', side_effect=lambda args, **kw: {'Package': 'linwin', 'Architecture': 'amd64', 'Version': self.version}[args[-1]] + '\n')
        self.metadata.start()

    def tearDown(self):
        self.metadata.stop()
        self.temp.cleanup()

    def write_xml(self):
        ET.ElementTree(self.xml).write(self.root / 'ctest-results.xml')

    def run_collect(self):
        return collect(self.root, self.name, self.version)

    def test_success(self):
        evidence = self.run_collect()
        self.assertEqual(evidence['native_suites_passed'], 18)
        self.assertEqual(len(evidence['resolution_captures']), 4)
        self.assertFalse(evidence['visual_reference_accepted'])
        self.assertFalse(evidence['physical_xfce_certified'])

    def test_failed_native(self):
        ET.SubElement(self.xml[0], 'failure'); self.write_xml()
        with self.assertRaises(ValueError): self.run_collect()

    def test_skipped_native(self):
        ET.SubElement(self.xml[0], 'skipped'); self.write_xml()
        with self.assertRaises(ValueError): self.run_collect()

    def test_missing_suite(self):
        self.xml.remove(self.xml[0]); self.write_xml()
        with self.assertRaises(ValueError): self.run_collect()

    def test_checksum_mismatch(self):
        (self.root / self.name).write_bytes(b'changed')
        with self.assertRaises(ValueError): self.run_collect()

    def test_failed_updater(self):
        (self.root / 'updater-tests.txt').write_text('Ran 45 tests in 0.1s\nFAILED\n')
        with self.assertRaises(ValueError): self.run_collect()

    def test_missing_capture(self):
        (self.root / 'resolution-checks/neon-1920x1080-start.png').unlink()
        with self.assertRaises(FileNotFoundError): self.run_collect()

    def test_invalid_package_name(self):
        with self.assertRaises(ValueError): collect(self.root, '../other.deb', self.version)

    def test_wrong_version(self):
        with self.assertRaises(ValueError): collect(self.root, self.name, '1.0.0-1')


if __name__ == '__main__':
    unittest.main()
