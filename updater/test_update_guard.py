# SPDX-License-Identifier: GPL-3.0-or-later
import subprocess
import tempfile
import unittest
from unittest.mock import Mock
from update_guard import installation_plan, update_lock, confirm_plan


class GuardTests(unittest.TestCase):
    def test_lock_excludes_other_invocation(self):
        with tempfile.TemporaryDirectory() as d:
            with update_lock(d) as first:
                self.assertTrue(first)
                with update_lock(d) as second:
                    self.assertFalse(second)
            with update_lock(d) as third:
                self.assertTrue(third)

    def test_lock_released_after_failure(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(RuntimeError):
                with update_lock(d):
                    raise RuntimeError('fixture')
            with update_lock(d) as acquired:
                self.assertTrue(acquired)

    def test_apt_plan_uses_simulation(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 0, 'Inst linwin (1.0.0-9)\nConf linwin (1.0.0-9)', ''))
        self.assertIn('Inst linwin', installation_plan('/tmp/linwin.deb', runner))
        runner.assert_called_once_with('/usr/bin/apt-get', '-s', '--no-remove', '--no-install-recommends', 'install', '/tmp/linwin.deb')

    def test_apt_failure(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 100, '', 'Unmet dependencies'))
        with self.assertRaisesRegex(ValueError, 'Unmet dependencies'):
            installation_plan('/tmp/linwin.deb', runner)

    def test_apt_removal(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 0, 'Remv desktop\nInst linwin (1.0.0-9)', ''))
        with self.assertRaisesRegex(ValueError, 'removing'):
            installation_plan('/tmp/linwin.deb', runner)

    def test_apt_no_linwin(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 0, 'Inst another-app (1.0)', ''))
        with self.assertRaisesRegex(ValueError, 'did not propose'):
            installation_plan('/tmp/linwin.deb', runner)

    def test_apt_preview_bounded(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 0, 'Inst linwin (1.0.0-9)\n' + 'Conf dependency\n'*40, ''))
        with self.assertRaisesRegex(ValueError, 'too many'):
            installation_plan('/tmp/linwin.deb', runner)

    def test_approved_plan_unchanged(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 0, 'Inst linwin (1.0.0-9)', ''))
        confirm_plan('/tmp/linwin.deb', runner, 'Inst linwin (1.0.0-9)')

    def test_approved_plan_changed(self):
        runner = Mock(return_value=subprocess.CompletedProcess([], 0, 'Inst linwin (1.0.0-9)\nInst dependency (2.0)', ''))
        with self.assertRaisesRegex(ValueError, 'changed after approval'):
            confirm_plan('/tmp/linwin.deb', runner, 'Inst linwin (1.0.0-9)')


if __name__ == '__main__':
    unittest.main(verbosity=2)
