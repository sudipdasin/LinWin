#!/usr/bin/python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Private JSON-lines bridge for the native LinWin update dialog."""
import argparse
import importlib.machinery
import importlib.util
import json
from pathlib import Path
import sys


def exchange(kind, message, input_stream=None, output_stream=None):
    if kind not in ('question', 'info', 'error'):
        raise ValueError('Unsupported update event')
    input_stream = sys.stdin if input_stream is None else input_stream
    output_stream = sys.stdout if output_stream is None else output_stream
    print(json.dumps({'kind': kind, 'message': str(message)}, ensure_ascii=True), file=output_stream, flush=True)
    return input_stream.readline(16) == 'yes\n' if kind == 'question' else True


def load_engine():
    loader = importlib.machinery.SourceFileLoader('linwin_update_engine', str(Path(__file__).with_name('linwin-update')))
    spec = importlib.util.spec_from_loader(loader.name, loader)
    engine = importlib.util.module_from_spec(spec)
    loader.exec_module(engine)
    return engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--channel', choices=('stable', 'preview'), required=True)
    args = parser.parse_args()
    engine = load_engine()
    engine.gui = exchange
    sys.argv = [sys.argv[0], '--channel', args.channel]
    return engine.main()


if __name__ == '__main__':
    sys.exit(main())
