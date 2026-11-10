#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
"""Create a verified, project-local patched core; retain the installed original."""

import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def files(directory):
    return {p.relative_to(directory): p for p in directory.rglob('*') if p.is_file()}


def main():
    root = Path(__file__).resolve().parent.parent
    subprocess.run(['python3', str(root / 'scripts/verify_arduino_core.py')], check=True)
    data = root / '.local/arduino-data'
    derived = root / '.local/sfc-arduino-data'
    rel = Path('packages/UIAP_HID/hardware/ch32v/1.2.15')
    original = data / rel
    target = derived / rel
    patch = root / 'patches/uiap-1.2.15-coherent-keyboard.patch'
    manifest_path = root / '.local/sfc-core-manifest.json'
    permitted = {Path('cores/arduino/uiapusb.c'), Path('cores/arduino/uiapusb.h')}

    with tempfile.TemporaryDirectory(prefix='sfc-core-', dir=root / '.local') as temp:
        prepared = Path(temp) / 'platform'
        shutil.copytree(original, prepared)
        subprocess.run(['patch', '-p1', '--batch', '--forward', '--fuzz=0',
                        '-i', str(patch)], cwd=prepared, check=True)
        before, after = files(original), files(prepared)
        if before.keys() != after.keys():
            raise SystemExit('Unexpected files in the derived core')
        changed = {p for p in before if before[p].read_bytes() != after[p].read_bytes()}
        if changed != permitted:
            raise SystemExit('Unexpected derived-core changes: ' + str(changed))
        if target.exists():
            if not manifest_path.is_file():
                raise SystemExit('Refusing to replace an unmarked derived core')
            existing = files(target)
            identical = existing.keys() == after.keys() and all(
                existing[p].read_bytes() == after[p].read_bytes() for p in after)
            if not identical:
                shutil.rmtree(target)
                shutil.copytree(prepared, target)
        else:
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copytree(prepared, target)

    # Share only installed tools and index metadata, never a writable core tree.
    for source in list(data.glob('*.json')) + list(data.glob('*.sig')) + [
            data / 'packages/builtin', data / 'packages/UIAP_HID/tools']:
        link = derived / source.relative_to(data)
        link.parent.mkdir(parents=True, exist_ok=True)
        if not link.exists():
            link.symlink_to(source, target_is_directory=source.is_dir())

    config = json.loads((root / '.local/arduino-cli.yaml').read_text())
    config['directories']['data'] = str(derived)
    config['directories']['user'] = str(root / '.local/sfc-sketchbook')
    config['build_cache']['path'] = str(root / '.local/sfc-arduino-cache')
    (root / '.local/sfc-arduino-cli.yaml').write_text(json.dumps(config, indent=2) + '\n')
    manifest = {
        'base_package': 'UIAP_HID:ch32v@1.2.15',
        'source_commit': '9471a150c9837c9055ef62ef759a1a593eddf1b1',
        'patch': str(patch.relative_to(root)),
        'patch_sha256': sha(patch),
        'unchanged_files': len(before) - len(changed),
        'changed_files': {str(p): {'original_sha256': sha(original / p),
                                 'patched_sha256': sha(target / p)}
                          for p in sorted(changed)},
    }
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    print('PASS: dedicated SFC core has exactly two patched files; original core retained')


if __name__ == '__main__':
    main()
