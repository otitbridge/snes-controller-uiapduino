#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Package the verified v1.0.0 BIN, sources and notices; no upload or flashing."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parent.parent


def sha(data):
    return hashlib.sha256(data).hexdigest()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def main():
    if git('diff', '--name-only', 'HEAD').strip():
        raise SystemExit('Commit tracked changes before packaging a release.')
    version = 'v1.0.0'
    # Read release inputs from HEAD so untracked files cannot enter the bundle.
    def tracked(path):
        return git('show', 'HEAD:' + path)

    manifest = json.loads(tracked('releases/v1.0.0/manifest.json'))
    firmware = (ROOT / '.local/build/sfc-keyboard/snes_controller.ino.bin').read_bytes()
    if len(firmware) != manifest['size_bytes'] or sha(firmware) != manifest['sha256']:
        raise SystemExit('BIN does not match the hardware-verified release manifest.')
    upstream = json.loads(tracked('docs/evidence/p0-upstream-2026-11-10.json'))['platform']
    core_path = ROOT / '.local/arduino-downloads/packages' / upstream['archiveFileName']
    core = core_path.read_bytes()
    if sha(core) != upstream['checksum'].split(':')[1].lower():
        raise SystemExit('Upstream core archive checksum mismatch.')

    commit = git('rev-parse', 'HEAD').decode().strip()
    manifest['source_commit'] = commit
    manifest['source_url'] = 'https://github.com/otitbridge/snes-controller-uiapduino/tree/' + commit
    files = {
        manifest['filename']: firmware,
        'README.md': tracked('releases/v1.0.0/README.md'),
        'setup_uiapflash.sh': tracked('scripts/setup_uiapflash.sh'),
        'LICENSE': tracked('LICENSE'),
        'THIRD_PARTY_NOTICES.md': tracked('releases/THIRD_PARTY_NOTICES.md'),
        'manifest.json': (json.dumps(manifest, indent=2) + '\n').encode(),
        'source/project-source.tar.gz': git('archive', '--format=tar.gz',
                                           '--prefix=snes-controller-uiapduino/', commit),
        'source/' + upstream['archiveFileName']: core,
    }
    # Preserve the original notices alongside the complete upstream source.
    notices = []
    with zipfile.ZipFile(core_path) as archive:
        for name in sorted(archive.namelist()):
            if name.endswith('/'):
                continue
            rel = name.split('/', 1)[-1]
            if rel == 'LICENSE':
                files['licenses/UIAPduino-core-MIT.txt'] = archive.read(name)
            if not name.endswith(('.c', '.h', '.cpp', '.S', '.s')):
                continue
            text = archive.read(name).decode('utf-8', errors='replace')
            for block in re.findall(r'/\*.*?\*/', text, re.DOTALL):
                if re.search(r'copyright|license|redistribution', block, re.I):
                    notices.append(rel + '\n' + block)
    files['licenses/upstream-source-notices.txt'] = ('\n\n'.join(notices) + '\n').encode()
    for name in ('LGPL-2.1.txt', 'GPL-3.0.txt', 'GCC-RUNTIME-EXCEPTION-3.1.txt',
                 'rv003usb-MIT.txt', 'WCH-BSD-3-Clause.txt'):
        files['licenses/' + name] = tracked('releases/licenses/' + name)
    files['SHA256SUMS'] = ''.join(sha(data) + '  ' + name + '\n'
                                 for name, data in sorted(files.items())).encode()
    output = ROOT / '.local/releases' / version
    output.mkdir(parents=True, exist_ok=True)
    bundle_name = 'snes-controller-uiapduino-' + version
    zip_path = output / (bundle_name + '.zip')
    with zipfile.ZipFile(zip_path, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(files.items()):
            info = zipfile.ZipInfo(bundle_name + '/' + name, (2026, 11, 11, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            archive.writestr(info, data)
    (output / manifest['filename']).write_bytes(firmware)
    (output / 'SHA256SUMS').write_text(''.join(
        sha(path.read_bytes()) + '  ' + path.name + '\n'
        for path in (output / manifest['filename'], zip_path)))
    print('Packaged ' + version + ' from ' + commit)
    print('BIN matches hardware-verified SHA-256; complete source and notices included.')
    print('Output: .local/releases/' + version)


if __name__ == '__main__':
    main()
