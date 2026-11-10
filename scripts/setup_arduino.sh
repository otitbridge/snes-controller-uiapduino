#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
set -euo pipefail

snes_project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
cd "$snes_project_dir"
if [[ "$(uname -s)" != Darwin ]]; then
  printf 'This pinned setup helper supports macOS; see docs/build.md.\n' >&2
  exit 1
fi

# Use the Intel CLI so its dependency selection matches the core's macOS tools.
snes_cli_archive=arduino-cli_1.5.1_macOS_64bit.tar.gz
mkdir -p .local/downloads .local/bin .local/logs
if [[ ! -f ".local/downloads/$snes_cli_archive" ]]; then
  curl --fail --location --connect-timeout 15 --max-time 180 \
    "https://github.com/arduino/arduino-cli/releases/download/v1.5.1/$snes_cli_archive" \
    --output ".local/downloads/$snes_cli_archive"
fi
printf '%s  %s\n' \
  c982e940027996bea9901050e95fae99c59c1dcfee54beedecaf28141e7bf2e7 \
  ".local/downloads/$snes_cli_archive" | shasum -a 256 -c -
tar -xzf ".local/downloads/$snes_cli_archive" -C .local/bin arduino-cli

python3 - <<'PY'
from pathlib import Path
import json
root = Path.cwd()
config = {
    'board_manager': {'additional_urls': [
        'https://github.com/tarosay/board_manager_files/raw/main/package_uiap_hid_index.json'
    ]},
    'directories': {
        'data': str(root / '.local/arduino-data'),
        'downloads': str(root / '.local/arduino-downloads'),
        'user': str(root / '.local/sketchbook'),
    },
    'build_cache': {'path': str(root / '.local/arduino-cache')},
    'updater': {'enable_notification': False},
}
# JSON is also valid YAML. Explicit config avoids the default global data path.
(root / '.local/arduino-cli.yaml').write_text(json.dumps(config, indent=2) + '\n')
PY

snes_cli=(.local/bin/arduino-cli --config-file .local/arduino-cli.yaml --no-color)
"${snes_cli[@]}" version
if [[ ! -f .local/arduino-data/packages/UIAP_HID/hardware/ch32v/1.2.15/platform.txt ]]; then
  "${snes_cli[@]}" core update-index
  "${snes_cli[@]}" core install UIAP_HID:ch32v@1.2.15
fi
python3 scripts/verify_arduino_core.py
"${snes_cli[@]}" core list
