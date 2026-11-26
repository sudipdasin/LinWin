# SPDX-License-Identifier: GPL-3.0-or-later
import unittest
from release_policy import BASE, select_release, version_key, beta_package_version, verify_release_version


def release(tag, signed=False):
    names = [f'linwin-{tag.removeprefix("v")}-ubuntu-24.04.deb', 'SHA256SUMS']
    if signed:
        names.append('SHA256SUMS.sig')
    return dict(tag_name=tag, draft=False, prerelease='-' in tag, published_at='2026-11-26', assets=[dict(name=n, browser_download_url=BASE+tag+'/'+n) for n in names])


class PolicyTests(unittest.TestCase):
    def test_numeric_order(self):
        self.assertGreater(version_key('v1.0.0-beta.10'), version_key('v1.0.0-beta.8'))
    def test_invalid_tags(self):
        for tag in [None, 2, 'v01.0.0', 'v1.0.0-beta.0', '../v1.0.0', 'v'+'9'*100]:
            self.assertIsNone(version_key(tag))
    def test_signature_required(self):
        self.assertEqual(select_release([release('v1.0.0-beta.8', True), release('v1.0.0-beta.9')], 'ubuntu-24.04', 'preview', True)[0], 'v1.0.0-beta.8')
    def test_no_signed(self):
        with self.assertRaises(ValueError): select_release([release('v1.0.0-beta.8')], 'ubuntu-24.04', 'preview', True)
    def test_duplicate_asset(self):
        r = release('v1.0.0-beta.8'); r['assets'].append(r['assets'][0])
        with self.assertRaises(ValueError): select_release([r], 'ubuntu-24.04', 'preview')
    def test_duplicate_tag(self):
        with self.assertRaises(ValueError): select_release([release('v1.0.0-beta.8'), release('1.0.0-beta.8')], 'ubuntu-24.04', 'preview')
    def test_mismatched_flag(self):
        r = release('v1.0.0-beta.8'); r['prerelease'] = False
        with self.assertRaises(ValueError): select_release([r], 'ubuntu-24.04', 'preview')
    def test_wrong_url(self):
        r = release('v1.0.0-beta.8'); r['assets'][0]['browser_download_url'] = 'https://github.com/another/repo'
        with self.assertRaises(ValueError): select_release([r], 'ubuntu-24.04', 'preview')
    def test_platform_and_channel(self):
        for suffix, channel in [('other','preview'), ('ubuntu-24.04','other')]:
            with self.assertRaises(ValueError): select_release([], suffix, channel)
    def test_beta_package(self):
        self.assertEqual(beta_package_version('v1.0.0-beta.10'), '1.0.0-10')
        verify_release_version('v1.0.0-beta.10', '1.0.0-10')
        with self.assertRaises(ValueError): verify_release_version('v1.0.0-beta.10', '1.0.0-9')
    def test_upstream_package(self):
        verify_release_version('v2.0.0', '2.0.0-1')
        with self.assertRaises(ValueError): verify_release_version('v2.0.0', '3.0.0-1')


if __name__ == '__main__': unittest.main(verbosity=2)
