import importlib.machinery
import importlib.util
import hashlib
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

loader = importlib.machinery.SourceFileLoader('update', str(Path(__file__).with_name('linwin-update')))
spec = importlib.util.spec_from_loader(loader.name, loader)
u = importlib.util.module_from_spec(spec)
loader.exec_module(u)


def release(tag, suffix='ubuntu-24.04', **kw):
    names = [f'linwin-{tag.removeprefix("v")}-{suffix}.deb', 'SHA256SUMS']
    return dict(tag_name=tag, published_at='2026-11-26T07:46:46Z', prerelease='-' in tag, draft=False,
                assets=[dict(name=n, browser_download_url=u.BASE+tag+'/'+n) for n in names]) | kw


class Tests(unittest.TestCase):
    def test_numeric_beta(self):
        self.assertGreater(u.version_key('v1.0.0-beta.10'), u.version_key('v1.0.0-beta.8'))
    def test_stable_after_beta(self):
        self.assertGreater(u.version_key('v1.0.0'), u.version_key('v1.0.0-rc.9'))
    def test_invalid_tags(self):
        for tag in ['latest','v1.0','../../evil','v1.0.0-beta.x']:
            self.assertIsNone(u.version_key(tag))
    def test_preview(self):
        self.assertEqual(u.select_release([release('v1.0.0-beta.8'),release('v1.0.0-beta.10')], 'ubuntu-24.04','preview')[0], 'v1.0.0-beta.10')
    def test_stable(self):
        self.assertEqual(u.select_release([release('v1.0.0'),release('v1.1.0-beta.1')], 'ubuntu-24.04','stable')[0], 'v1.0.0')
    def test_draft_skipped(self):
        self.assertEqual(u.select_release([release('v1.0.0'),release('v2.0.0',draft=True)], 'ubuntu-24.04','preview')[0], 'v1.0.0')
    def test_wrong_distro_skipped(self):
        self.assertEqual(u.select_release([release('v1.0.0'),release('v2.0.0','debian-13')], 'ubuntu-24.04','preview')[0], 'v1.0.0')
    def test_no_manifest(self):
        with self.assertRaises(ValueError): u.select_release([release('v1.0.0',assets=[])], 'ubuntu-24.04','preview')
    def test_mint(self):
        self.assertEqual(u.platform_suffix({'ID':'linuxmint','VERSION_ID':'22.3','UBUNTU_CODENAME':'noble'},'amd64'),'ubuntu-24.04')
    def test_debian(self):
        self.assertEqual(u.platform_suffix({'ID':'debian','VERSION_ID':'13'},'amd64'),'debian-13')
    def test_bad_arch(self):
        with self.assertRaises(ValueError): u.platform_suffix({'ID':'ubuntu','VERSION_ID':'24.04'},'arm64')
    def test_bad_distro(self):
        with self.assertRaises(ValueError): u.platform_suffix({'ID':'ubuntu','VERSION_ID':'22.04'},'amd64')
    def test_host(self):
        self.assertTrue(u.trusted_url('https://release-assets.githubusercontent.com/x'))
        for url in ['http://github.com/x','https://github.com.evil.net/x','https://evil@github.com/x','https://github.com:80/x']:
            self.assertFalse(u.trusted_url(url))
    def test_asset(self):
        with self.assertRaises(ValueError): u.asset_url('v1.0.0','SHA256SUMS',{'browser_download_url':'https://github.com/other/release'})
    def test_hash(self):
        h=hashlib.sha256(b'data').hexdigest()
        self.assertEqual(u.expected_hash(f'{h}  file.deb\n'.encode(),'file.deb'),h)
        with self.assertRaises(ValueError): u.expected_hash(f'{h}  file.deb\n{h}  file.deb\n'.encode(),'file.deb')
        with self.assertRaises(ValueError): u.expected_hash(f'{h}  other.deb\n'.encode(),'file.deb')
    def test_tamper(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'file'; p.write_bytes(b'original')
            u.verify_file(p,hashlib.sha256(b'original').hexdigest())
            p.write_bytes(b'tampered')
            with self.assertRaises(ValueError): u.verify_file(p,hashlib.sha256(b'original').hexdigest())
    def test_dpkg_version(self):
        self.assertTrue(u.newer('1.0.0-10','1.0.0-8'))
        self.assertFalse(u.newer('1.0.0-8','1.0.0-8'))
        self.assertFalse(u.newer('1.0.0-7','1.0.0-8'))
    def test_package_metadata(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)/'root'; (root/'DEBIAN').mkdir(parents=True)
            for name,arch,accepted in [('linwin','amd64',True),('other','amd64',False),('linwin','arm64',False)]:
                (root/'DEBIAN/control').write_text(f'Package: {name}\nVersion: 1.0.0-9\nArchitecture: {arch}\nMaintainer: Test <test@example.invalid>\nDescription: fixture\n')
                pkg=Path(d)/'test.deb'
                subprocess.run(['dpkg-deb','--build',str(root),str(pkg)],check=True,stdout=subprocess.DEVNULL)
                if accepted:
                    self.assertEqual(u.inspect_deb(pkg,'1.0.0-8'),('1.0.0-9',True))
                    self.assertEqual(u.inspect_deb(pkg,'1.0.0-10'),('1.0.0-9',False))
                else:
                    with self.assertRaises(ValueError): u.inspect_deb(pkg,'1.0.0-8')
    def test_background_never_installs(self):
        with tempfile.TemporaryDirectory() as d, patch.dict(u.os.environ,{'XDG_CACHE_HOME':d}), patch.object(u,'installed_version',return_value='1.0.0-8'), patch.object(u,'system_suffix',return_value='ubuntu-24.04'), patch.object(u,'releases',return_value=[release('v1.0.0-beta.9')]), patch.object(u,'notify') as notify, patch.object(u,'fetch',side_effect=AssertionError('unexpected network')), patch.object(u,'gui',side_effect=AssertionError('unexpected prompt')), patch.object(u,'newer',return_value=True), patch.object(u,'run',side_effect=AssertionError('unexpected command')):
            (Path(d)/'linwin-update').mkdir()
            u.check('preview',True);u.check('preview',True)
            self.assertEqual(notify.call_count,1)

if __name__ == '__main__': unittest.main(verbosity=2)
