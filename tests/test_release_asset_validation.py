import unittest

from scripts.validate_release_assets import (
    expected_assets,
    expected_deb_version,
    validate_deb,
    validate_release,
)


TAG = "v1.0.0-beta.8"


def release_fixture():
    return {
        "tag_name": TAG,
        "assets": [
            {
                "name": name,
                "browser_download_url": f"https://github.com/sudipdasin/LinWin/releases/download/{TAG}/{name}",
            }
            for name in sorted(expected_assets(TAG))
        ],
    }


class ReleaseAssetValidationTests(unittest.TestCase):
    def test_release_asset_set_is_complete_and_urls_match(self):
        self.assertEqual(validate_release(release_fixture(), TAG), [])

    def test_missing_asset_is_reported(self):
        release = release_fixture()
        release["assets"].pop()
        self.assertTrue(any("Missing release asset:" in error for error in validate_release(release, TAG)))

    def test_wrong_asset_url_is_reported(self):
        release = release_fixture()
        release["assets"][0]["browser_download_url"] = "https://example.invalid/wrong.deb"
        self.assertTrue(any("Wrong download URL" in error for error in validate_release(release, TAG)))

    def test_beta7_release_with_no_assets_is_reported(self):
        release = {"tag_name": "v1.0.0-beta.7", "assets": []}
        errors = validate_release(release, "v1.0.0-beta.7")
        self.assertEqual(len(errors), len(expected_assets("v1.0.0-beta.7")))

    def test_beta_tag_maps_to_exact_debian_package_version(self):
        self.assertEqual(expected_deb_version("v1.0.0-beta.8"), "1.0.0-8")

    def test_ubuntu_mint_dependencies_accept_qt_64_package(self):
        depends = (
            "libc6 (>= 2.34), libqt6core6t64 (>= 6.4.2), "
            "libqt6gui6t64 (>= 6.1.2), libqt6widgets6t64 (>= 6.3.0), libstdc++6 (>= 5)"
        )
        self.assertEqual(validate_deb("ubuntu.deb", "ubuntu", depends), [])

    def test_ubuntu_mint_rejects_debian_13_qt_requirement(self):
        depends = (
            "libc6 (>= 2.34), libqt6core6t64 (>= 6.8.2), "
            "libqt6gui6t64 (>= 6.1.2), libqt6widgets6t64 (>= 6.3.0), libstdc++6 (>= 5)"
        )
        self.assertTrue(any("provides Qt 6.4.2" in error for error in validate_deb("ubuntu.deb", "ubuntu", depends)))

    def test_debian_13_dependencies_are_checked_separately(self):
        valid = "libqt6core6t64 (>= 6.8.2), libqt6gui6 (>= 6.1.2), libqt6widgets6 (>= 6.3.0), libstdc++6 (>= 14)"
        self.assertEqual(validate_deb("debian.deb", "debian", valid), [])
        invalid = "libqt6core6t64 (>= 6.4.0), libqt6gui6t64, libqt6widgets6t64, libstdc++6 (>= 13)"
        self.assertEqual(len(validate_deb("debian.deb", "debian", invalid)), 3)


if __name__ == "__main__":
    unittest.main()
