#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
# Standalone installer: downloads the upstream uploader; never writes a board.
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
  printf 'This helper supports macOS only.\n' >&2
  exit 1
fi
if [[ $# -gt 1 ]]; then
  printf 'Usage: bash setup_uiapflash.sh [destination-directory]\n' >&2
  exit 2
fi
snes_destination="${1:-.local/uiapflash}"
mkdir -p "$snes_destination"
snes_destination="$(cd "$snes_destination" && pwd -P)"
snes_temp="$(mktemp -d "$snes_destination/.download.XXXXXX")"
trap 'rm -rf "$snes_temp"' EXIT

curl --fail --location --proto '=https' --proto-redir '=https' \
  --connect-timeout 15 --max-time 180 \
  https://github.com/tarosay/uiapflash/releases/download/v1.0.0/uiapflash-1.0.0-macos.tar.gz \
  --output "$snes_temp/uiapflash.tar.gz"
printf '%s  %s\n' \
  185e9c3ea69eba278c4c4b88b4cc5304be986a4edaf0548041fdb39ddf740d01 \
  "$snes_temp/uiapflash.tar.gz" | shasum -a 256 -c -
tar -xzf "$snes_temp/uiapflash.tar.gz" -C "$snes_temp" \
  uiapflash-1.0.0/uiapflash uiapflash-1.0.0/LICENSE uiapflash-1.0.0/README.md
printf '%s  %s\n' \
  84381d22d24eb49c478a319ec6fb94af1702d718745757cd72d51eb476c29210 \
  "$snes_temp/uiapflash-1.0.0/uiapflash" | shasum -a 256 -c -
for snes_file in uiapflash LICENSE README.md; do
  cp "$snes_temp/uiapflash-1.0.0/$snes_file" "$snes_destination/$snes_file"
done
chmod +x "$snes_destination/uiapflash"
printf 'Ready: %s/uiapflash (1.0.0, macOS arm64/x86_64)\nNo firmware was written.\n' "$snes_destination"
