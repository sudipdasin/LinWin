import importlib.util
import unittest
from pathlib import Path
from unittest.mock import patch


MODULE_PATH = Path(__file__).parents[1] / "scripts" / "linwin_updater.py"
spec = importlib.util.spec_from_file_location("linwin_updater", MODULE_PATH)
updater = importlib.util.module_from_spec(spec)
spec.loader.exec_module(updater)


def release(tag, files):
    assets = []
    for filename in files:
        assets.append({
            "name": filename,
            "browser_download_url": f"{updater.DOWNLOAD_ROOT}/{tag}/{filename}",
        })
    return {"tag_name": tag, "draft": False, "assets": assets}


class UpdaterTests(unittest.TestCase):
    def test_platform_routing_is_narrow_and_uses_mint_ubuntu_package(self):
        self.assertEqual(updater.package_target({"ID": "linuxmint", "VERSION_ID": "22.3"}), "ubuntu")
        self.assertEqual(updater.package_target({"ID": "ubuntu", "VERSION_ID": "24.04"}), "ubuntu")
        self.assertEqual(updater.package_target({"ID": "debian", "VERSION_ID": "13"}), "debian")
        with self.assertRaisesRegex(ValueError, "support Linux Mint 22"):
            updater.package_target({"ID": "debian", "VERSION_ID": "12"})

    def test_beta_versions_are_sorted_numerically(self):
        self.assertGreater(updater.tag_key("v1.0.0-beta.10"), updater.tag_key("v1.0.0-beta.9"))
        self.assertGreater(updater.tag_key("v1.0.0"), updater.tag_key("v1.0.0-beta.99"))
        self.assertEqual(updater.package_version_for_tag("v1.0.0-beta.10"), "1.0.0-10")

    def test_incomplete_beta7_is_skipped_for_complete_beta8(self):
        beta7 = release("v1.0.0-beta.7", [])
        beta8 = release("v1.0.0-beta.8", [
            "linwin-1.0.0-beta.8-ubuntu-24.04.deb", "SHA256SUMS"
        ])
        selected = updater.choose_release([beta8, beta7], "ubuntu")
        self.assertEqual(selected["tag"], "v1.0.0-beta.8")
        self.assertEqual(selected["asset"], "linwin-1.0.0-beta.8-ubuntu-24.04.deb")

    def test_stable_release_is_not_used_until_package_version_policy_is_defined(self):
        stable = release("v1.0.0", ["linwin-1.0.0-ubuntu-24.04.deb", "SHA256SUMS"])
        with self.assertRaisesRegex(ValueError, "No complete published"):
            updater.choose_release([stable], "ubuntu")

    def test_release_asset_url_must_match_tag_and_expected_filename(self):
        bad = release("v1.0.0-beta.8", [
            "linwin-1.0.0-beta.8-ubuntu-24.04.deb", "SHA256SUMS"
        ])
        bad["assets"][0]["browser_download_url"] = "https://attacker.invalid/linwin.deb"
        with self.assertRaisesRegex(ValueError, "No complete published"):
            updater.choose_release([bad], "ubuntu")

    def test_sha256_entry_is_unique_and_matches_filename(self):
        name = "linwin-1.0.0-beta.8-ubuntu-24.04.deb"
        digest = "a" * 64
        self.assertEqual(updater.expected_checksum(f"{digest}  {name}\n", name), digest)
        with self.assertRaisesRegex(ValueError, "exactly one"):
            updater.expected_checksum(f"{digest}  {name}\n{digest} *{name}\n", name)
        with self.assertRaisesRegex(ValueError, "exactly one"):
            updater.expected_checksum(f"{digest}  another.deb\n", name)

    def test_package_digest_must_match_release_checksum(self):
        name = "linwin-1.0.0-beta.8-ubuntu-24.04.deb"
        data = b"package bytes"
        digest = updater.hashlib.sha256(data).hexdigest()
        updater.verify_checksum(data, f"{digest}  {name}\n", name)
        with self.assertRaisesRegex(ValueError, "does not match"):
            updater.verify_checksum(data + b"tampered", f"{digest}  {name}\n", name)

    def test_target_specific_package_dependencies(self):
        ubuntu = (
            "libqt6core6t64 (>= 6.4.0), libqt6gui6t64 (>= 6.1.2), "
            "libqt6widgets6t64 (>= 6.3.0), libstdc++6 (>= 5)"
        )
        debian = (
            "libqt6core6t64 (>= 6.8.2), libqt6gui6 (>= 6.1.2), "
            "libqt6widgets6 (>= 6.3.0), libstdc++6 (>= 14)"
        )
        updater.validate_target_dependencies("ubuntu", ubuntu)
        updater.validate_target_dependencies("debian", debian)
        with self.assertRaisesRegex(ValueError, "supported range"):
            updater.validate_target_dependencies("ubuntu", debian)
        with self.assertRaisesRegex(ValueError, "expected at least 6.8.2"):
            updater.validate_target_dependencies("debian", ubuntu)

    @patch.object(updater.subprocess, "run")
    def test_update_comparison_uses_debian_version_ordering(self, run):
        run.return_value.returncode = 0
        self.assertTrue(updater.is_newer("1.0.0-10", "1.0.0-9"))
        self.assertEqual(run.call_args.args[0], ["/usr/bin/dpkg", "--compare-versions", "1.0.0-9", "lt", "1.0.0-10"])


if __name__ == "__main__":
    unittest.main()
