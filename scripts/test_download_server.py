import tempfile
import threading
import unittest
from functools import partial
from http.server import ThreadingHTTPServer
from pathlib import Path
from urllib.error import HTTPError
from urllib.request import Request, urlopen
from download_server import DownloadHandler


class DownloadTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        root = Path(self.temp.name)
        self.name = 'linwin-1.0.1-preview.6-ubuntu-24.04-amd64.deb'
        (root / self.name).write_bytes(b'fixture bytes')
        (root / 'linwin-latest.deb').symlink_to(self.name)
        (root / 'index.html').write_text('<h1>LinWin</h1>')
        (root / 'empty').mkdir()
        self.server = ThreadingHTTPServer(('127.0.0.1', 0), partial(DownloadHandler, directory=self.temp.name))
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.base = f'http://127.0.0.1:{self.server.server_port}'

    def tearDown(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join()
        self.temp.cleanup()

    def test_latest_and_versioned_get(self):
        for name in ('linwin-latest.deb', self.name):
            with urlopen(self.base + '/' + name) as r:
                self.assertEqual(r.status, 200)
                self.assertEqual(r.geturl(), self.base + '/' + name)
                self.assertEqual(r.headers['Content-Disposition'], f'attachment; filename="{self.name}"')
                self.assertEqual(r.headers['Content-Type'], 'application/vnd.debian.binary-package')
                self.assertEqual(r.read(), b'fixture bytes')

    def test_head(self):
        with urlopen(Request(self.base + '/linwin-latest.deb', method='HEAD')) as r:
            self.assertEqual(r.headers['Content-Length'], '13')
            self.assertEqual(r.headers['Cache-Control'], 'no-store')
            self.assertEqual(r.read(), b'')

    def test_gallery(self):
        with urlopen(self.base + '/') as r:
            self.assertIsNone(r.headers['Content-Disposition'])
            self.assertEqual(r.read(), b'<h1>LinWin</h1>')

    def test_missing_and_listing(self):
        for path in ('/linwin-missing.deb', '/empty/'):
            with self.assertRaises(HTTPError) as err:
                urlopen(self.base + path)
            self.assertEqual(err.exception.code, 404)
            self.assertIsNone(err.exception.headers['Content-Disposition'])


if __name__ == '__main__':
    unittest.main()
