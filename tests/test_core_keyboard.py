#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Exercise actual core code with ISR reads between writer statements."""

import os
from pathlib import Path
import re
import subprocess


def extract_function(source, name):
    start = source.index('void ' + name + '(')
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def extract(source):
    start = source.index('static volatile uint8_t keyboard_report')
    storage = source[start:source.index('#ifdef UIAP_WEBHID', start)]
    writer = extract_function(source, 'uiapkbd_keyboard_set')
    # Inject after each actual statement, including publication. The ISR can
    # only observe memory changes at stores; no target interrupt masking added.
    writer = '\n'.join(line + ('\n    test_interrupt();' if ';' in line else '')
                       for line in writer.splitlines())
    clear = extract_function(source, 'uiapkbd_keyboard_clear')
    start = source.index('} else if (endp == 2) {') + len('} else if (endp == 2) {')
    reader = source[start:source.index('#ifdef UIAP_WEBHID', start)]
    return storage + writer + '\n' + clear + '\nstatic void read_keyboard(void) {\n' + \
        '    const uint32_t sendtok = 0x4b;\n' + reader + '\n}\n'


def main():
    root = Path(__file__).resolve().parent.parent
    relative = Path('packages/UIAP_HID/hardware/ch32v/1.2.15/cores/arduino/uiapusb.c')
    work = root / '.local/host-tests/core-keyboard'
    work.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic', '-O2']
    if os.environ.get('SNES_HOST_SANITIZERS') == '1':
        flags += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    for label, directory in [('upstream', 'arduino-data'), ('patched', 'sfc-arduino-data')]:
        source = (root / '.local' / directory / relative).read_text()
        (work / 'tested_core_keyboard.h').write_text(extract(source))
        binary = work / label
        subprocess.run([os.environ.get('CC', 'cc')] + flags + ['-I', str(work),
                        str(root / 'tests/core_keyboard_test.c'), '-o', str(binary)], check=True)
        result = subprocess.run([str(binary)], text=True, capture_output=True)
        if label == 'upstream':
            if result.returncode != 1 or 'Mixed keyboard snapshot' not in result.stderr:
                raise SystemExit('Upstream did not reproduce the expected torn-state regression')
            print('PASS: upstream setter reproduces a mixed state under injected interruption')
        else:
            if result.returncode:
                raise SystemExit(result.stderr or 'Patched-core snapshot test failed')
            print(result.stdout, end='')


if __name__ == '__main__':
    main()
