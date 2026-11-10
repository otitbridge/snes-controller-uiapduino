#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Bounded macOS EP0 Input sampling. Does not capture interrupt-IN packets."""

import json
import datetime
from pathlib import Path
import signal
import subprocess
import sys
import time


def main():
    trace = len(sys.argv) == 3 and sys.argv[2] == '--trace'
    if (len(sys.argv) not in (2, 3) or (len(sys.argv) == 3 and not trace) or
            not sys.argv[1].isdigit() or not 1 <= int(sys.argv[1]) <= 600):
        raise SystemExit('Usage: python3 scripts/read_keyboard_control.py <seconds: 1..600> [--trace]')
    root = Path(__file__).resolve().parent.parent
    binary = root / '.local/bin/read_keyboard_control'
    binary.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                    '-Wno-deprecated-declarations', '-framework', 'IOKit',
                    '-framework', 'CoreFoundation', str(root / 'scripts/read_keyboard_control.c'),
                    '-o', str(binary)], check=True)
    stopping = False

    def stop(*_):
        nonlocal stopping
        stopping = True

    signal.signal(signal.SIGINT, stop)
    signal.signal(signal.SIGTERM, stop)
    started = time.monotonic()
    samples = errors = 0
    print(json.dumps({'event': 'ready', 'duration_seconds': int(sys.argv[1]),
                      'at': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      'request': {'bmRequestType': 193 if trace else 161,
                                  'bRequest': 90 if trace else 1,
                                  'wValue': 0 if trace else 256,
                                  'wIndex': 1, 'wLength': 8}}), flush=True)
    while not stopping and time.monotonic() - started < int(sys.argv[1]):
        try:
            command = [str(binary)] + (['--trace'] if trace else [])
            result = subprocess.run(command, capture_output=True, text=True, timeout=3)
            if result.returncode:
                row = {'event': 'unavailable', 'exit_code': result.returncode,
                       'stderr': result.stderr.strip(), 'response': result.stdout.strip()}
                errors += 1
                if result.returncode == 3 and 'found 0' not in result.stderr:
                    stopping = True  # More than one target: never choose one.
            else:
                row = json.loads(result.stdout)
                valid = (row.get('result') == '0x00000000' and row.get('length') == 8 and
                         len(row.get('bytes', [])) == 8)
                if trace:
                    valid = valid and row['bytes'][:2] == [84, 1] and row['bytes'][3] in (0, 1)
                if valid:
                    row['event'] = 'sample'
                    if trace:
                        data = row['bytes']
                        row.update(first_key=data[2], toggle=data[3],
                                   in_attempts=data[4] | (data[5] << 8),
                                   ack_count=data[6] | (data[7] << 8))
                    samples += 1
                else:
                    row.update(event='unavailable', reason='Unexpected response length or trace format')
                    errors += 1
        except subprocess.TimeoutExpired:
            row = {'event': 'unavailable', 'reason': '3-second subprocess timeout'}
            errors += 1
        row['seconds'] = round(time.monotonic() - started, 6)
        row['at'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        print(json.dumps(row), flush=True)
        if not stopping:
            time.sleep(0.2)
    print(json.dumps({'event': 'finished', 'samples': samples,
                      'unavailable': errors}), flush=True)


if __name__ == '__main__':
    main()
