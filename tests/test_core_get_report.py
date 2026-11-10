#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Test actual pure KBD+Mouse configuration, class routing and Input callbacks."""

import os
from pathlib import Path
import subprocess

from test_core_keyboard import extract_function


def extract(core, trace=False):
    source = (core / 'uiapusb.c').read_text()
    start = source.index('static volatile uint8_t mouse_report')
    storage = source[start:source.index('#ifdef UIAP_WEBHID', start)]
    functions = '\n'.join(extract_function(source, name) for name in [
        'uiapkbd_keyboard_set', 'uiapkbd_keyboard_clear', 'uiapkbd_mouse_set'])
    marker = '#ifndef UIAP_WEBHID\n/* Project patch: GET_REPORT(Input)'
    if marker in source:
        start = source.index(marker)
        functions += '\n' + source[start:source.index('#endif // !UIAP_WEBHID', start)]
        functions += '\n#endif\n'
    dispatch = (core / 'rv003usb.c').read_text()
    start = dispatch.index('if( reqShl == (0x0921>>1) )')
    end = dispatch.index('if( reqShl == (0x0680>>1) )', start)
    routing = dispatch[start:end] + '{}\n'
    # The compiler, rather than a Python model, evaluates the actual configuration.
    options = ['-DUIAP_SNES_USB_TRACE=1'] if trace else []
    macros = subprocess.run([os.environ.get('CC', 'cc'), '-x', 'c', '-E', '-dM'] + options + [
                             '-D__ASSEMBLER__', '-DUIAP_COMPOSITE_HID',
                             str(core / 'rv003usb.h')],
                            check=True, capture_output=True, text=True).stdout
    selected = '\n'.join(line for line in macros.splitlines()
                         if line.startswith(('#define RV003USB_', '#define ENDPOINTS ',
                                             '#define UIAP_SNES_USB_TRACE ')))
    if trace:
        start = source.index('} else if (endp == 2) {') + len('} else if (endp == 2) {')
        reader = source[start:source.index('#ifdef UIAP_WEBHID', start)]
        functions += '\nstatic void test_keyboard_in(void) {\n' + \
            '    const uint32_t sendtok = 0x4b;\n' + reader + '\n}\n'
    return selected + '\n' + storage + '\n' + functions + \
        '\nstatic void dispatch_report(struct usb_endpoint *e, uint32_t request,\n' + \
        '                            int wLength, uint32_t wvi) {\n' + \
        '    struct { unsigned reboot_armed; } state = {0}, *ist = &state;\n' + \
        '    uint32_t reqShl = request >> 1;\n' + \
        '    (void)ist; (void)wLength; e->opaque = 0; e->max_len = 0;\n' + routing + '}\n'


def main():
    root = Path(__file__).resolve().parent.parent
    relative = Path('packages/UIAP_HID/hardware/ch32v/1.2.15/cores/arduino')
    work = root / '.local/host-tests/core-get-report'
    work.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic', '-O2']
    if os.environ.get('SNES_HOST_SANITIZERS') == '1':
        flags += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    for label, directory in [('upstream', 'arduino-data'), ('patched', 'sfc-arduino-data'),
                             ('trace', 'sfc-arduino-data')]:
        header = extract(root / '.local' / directory / relative, label == 'trace')
        (work / 'tested_core_get_report.h').write_text(header)
        binary = work / label
        subprocess.run([os.environ.get('CC', 'cc')] + flags + ['-I', str(work),
                        str(root / 'tests/core_get_report_test.c'), '-o', str(binary)], check=True)
        result = subprocess.run([str(binary)], text=True, capture_output=True)
        if label == 'upstream':
            if result.returncode != 1 or 'GET_REPORT(Input) returned no keyboard state' not in result.stderr:
                raise SystemExit('Upstream did not reproduce the missing GET_REPORT response')
            print('PASS: upstream pure KBD+Mouse GET_REPORT returns no keyboard state')
        elif result.returncode:
            raise SystemExit(result.stderr or 'Patched-core GET_REPORT test failed')
        else:
            print(result.stdout, end='')


if __name__ == '__main__':
    main()
