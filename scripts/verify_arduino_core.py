#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Check installed UIAPduino source and tool archives against the P0 manifest."""

import hashlib
import json
from pathlib import Path
import zipfile


def require(condition, message):
    if not condition:
        raise SystemExit(message)


def main():
    root = Path(__file__).resolve().parent.parent
    manifest = json.loads((root / 'docs/evidence/p0-upstream-2026-11-10.json').read_text())
    core = root / '.local/arduino-data/packages/UIAP_HID/hardware/ch32v/1.2.15'
    cache = root / '.local/arduino-downloads/packages'
    packages = [manifest['platform']]
    packages.extend(next(s for s in tool['systems'] if s['host'] == 'x86_64-apple-darwin')
                    for tool in manifest['macos_tools'])
    for package in packages:
        archive = cache / package['archiveFileName']
        require(archive.is_file(), 'Missing archive: ' + package['archiveFileName'])
        actual = hashlib.sha256(archive.read_bytes()).hexdigest()
        require(actual == package['checksum'].split(':', 1)[1].lower(),
                'Archive checksum mismatch: ' + package['archiveFileName'])

    with zipfile.ZipFile(cache / manifest['platform']['archiveFileName']) as archive:
        files = [entry for entry in archive.infolist() if not entry.is_dir()]
        for entry in files:
            rel = Path(*Path(entry.filename).parts[1:])
            require((core / rel).read_bytes() == archive.read(entry),
                    'Installed source differs from package: ' + str(rel))

    for source in manifest['inspected_files']:
        content = (core / source['path']).read_bytes()
        blob = b'blob ' + str(len(content)).encode() + b'\0' + content
        require(hashlib.sha1(blob).hexdigest() == source['blob_sha'],
                'Reference blob mismatch: ' + source['path'])

    print('PASS: six core/tool archives match recorded SHA-256 values')
    print('PASS: all %d packaged files match installed core; 12 reference blobs match' % len(files))


if __name__ == '__main__':
    main()
