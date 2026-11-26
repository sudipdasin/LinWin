#!/usr/bin/env python3
"""Serve the verified native build with explicit attachment metadata."""
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote, urlsplit
import os
import re

PACKAGE = re.compile(r'linwin-[0-9][A-Za-z0-9.+~_-]*\.deb\Z')


class DownloadHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        path = unquote(urlsplit(self.path).path)
        name = path.rsplit('/', 1)[-1]
        if self._status == 200 and '/' + name == path and (name == 'linwin-latest.deb' or PACKAGE.fullmatch(name)):
            file = Path(self.directory) / name
            target = file.resolve()
            if target.parent == Path(self.directory).resolve() and target.is_file() and PACKAGE.fullmatch(target.name):
                self.send_header('Content-Disposition', f'attachment; filename="{target.name}"')
        self.send_header('X-Content-Type-Options', 'nosniff')
        self.send_header('Cache-Control', 'no-store' if name in ('linwin-latest.deb', 'SHA256SUMS', 'index.html', '') else 'no-cache')
        super().end_headers()

    def send_response(self, code, message=None):
        self._status = code
        super().send_response(code, message)

    def guess_type(self, path):
        if path.endswith('.deb'):
            return 'application/vnd.debian.binary-package'
        return super().guess_type(path)

    def list_directory(self, path):
        self.send_error(404, 'Directory listing disabled')
        return None


def serve():
    server = ThreadingHTTPServer(('0.0.0.0', int(os.environ.get('PORT', '10000'))), partial(DownloadHandler, directory='/public'))
    server.serve_forever()


if __name__ == '__main__':
    serve()
