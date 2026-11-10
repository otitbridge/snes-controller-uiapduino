#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Serve the SFC keyboard browser test and record its events on loopback only."""

from http.server import BaseHTTPRequestHandler, HTTPServer
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent.parent
ALLOWED_CODES = {'KeyZ', 'KeyA', 'Backspace', 'Enter', 'ArrowUp', 'ArrowDown',
                 'ArrowLeft', 'ArrowRight', 'KeyX', 'KeyS', 'KeyQ', 'KeyW'}
ALLOWED_EVENTS = {'start', 'keydown', 'keyup', 'focus_lost', 'window_blur'}


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def do_GET(self):
        if self.path != '/':
            self.send_error(404)
            return
        body = (ROOT / 'tests/sfc_keyboard_probe.html').read_bytes()
        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.send_header('Content-Length', str(len(body)))
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        origin = 'http://127.0.0.1:' + str(self.server.server_port)
        if self.path != '/events' or self.headers.get('Origin') != origin:
            self.send_error(403)
            return
        try:
            length = int(self.headers.get('Content-Length', '0'))
            if not 0 < length <= 8192:
                raise ValueError('length')
            row = json.loads(self.rfile.read(length))
            if row['event'] not in ALLOWED_EVENTS or len(row['session']) > 64:
                raise ValueError('event/session')
            if row.get('code') and row['code'] not in ALLOWED_CODES:
                raise ValueError('code')
            if any(code not in ALLOWED_CODES for code in row['pressed']):
                raise ValueError('pressed')
        except (ValueError, KeyError, TypeError):
            self.send_error(400)
            return
        with self.server.log_path.open('a') as log:
            log.write(json.dumps(row, ensure_ascii=False) + '\n')
        print(json.dumps(row, ensure_ascii=False), flush=True)
        self.send_response(204)
        self.end_headers()


def main():
    if len(sys.argv) != 2:
        raise SystemExit('Usage: python3 scripts/serve_keyboard_probe.py <new-log-name>')
    name = sys.argv[1]
    if Path(name).name != name or not name.endswith('.jsonl'):
        raise SystemExit('Supply a simple .jsonl filename, not a path')
    log_path = ROOT / '.local/logs' / name
    log_path.parent.mkdir(parents=True, exist_ok=True)
    if log_path.exists():
        raise SystemExit('Log already exists; choose a new name')
    log_path.touch()
    server = HTTPServer(('127.0.0.1', 0), Handler)
    server.log_path = log_path
    print(json.dumps({'event': 'ready', 'url': 'http://127.0.0.1:' + str(server.server_port),
                      'log': str(log_path.relative_to(ROOT))}), flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == '__main__':
    main()
