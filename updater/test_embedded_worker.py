# SPDX-License-Identifier: GPL-3.0-or-later
import io
import json
import unittest
from unittest.mock import patch
import embedded_worker as worker


class EmbeddedWorkerTests(unittest.TestCase):
    def test_yes(self):
        out = io.StringIO()
        self.assertTrue(worker.exchange('question', 'Install?\n<plain>', io.StringIO('yes\n'), out))
        self.assertEqual(json.loads(out.getvalue()), {'kind': 'question', 'message': 'Install?\n<plain>'})

    def test_denial_and_eof(self):
        for value in ('no\n', '', 'YES\n', 'yes please\n'):
            self.assertFalse(worker.exchange('question', 'Install?', io.StringIO(value), io.StringIO()))

    def test_information_does_not_read(self):
        inp = io.StringIO('yes\n')
        self.assertTrue(worker.exchange('info', 'Done', inp, io.StringIO()))
        self.assertEqual(inp.tell(), 0)

    def test_bad_event(self):
        with self.assertRaises(ValueError):
            worker.exchange('unknown', 'bad', io.StringIO(), io.StringIO())

    def test_engine_uses_native_bridge_and_channel(self):
        from unittest.mock import Mock
        engine = Mock()
        engine.main.return_value = 0
        with patch.object(worker, 'load_engine', return_value=engine), patch.object(worker.sys, 'argv', ['embedded_worker.py', '--channel', 'preview']):
            self.assertEqual(worker.main(), 0)
            self.assertIs(engine.gui, worker.exchange)
            self.assertEqual(worker.sys.argv[1:], ['--channel', 'preview'])
            engine.main.assert_called_once_with()

    def test_load_engine(self):
        self.assertTrue(callable(worker.load_engine().check))


if __name__ == '__main__':
    unittest.main()
