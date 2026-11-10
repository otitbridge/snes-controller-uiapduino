#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
set -euo pipefail

snes_project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
cd "$snes_project_dir"
snes_mode="${1:-neutral}"
case "$snes_mode" in
  neutral) snes_test_input=0 ;;
  manual) snes_test_input=1 ;;
  *) printf 'Usage: bash scripts/build_firmware.sh [neutral|manual]\n' >&2; exit 2 ;;
esac

snes_cli=.local/bin/arduino-cli
if [[ ! -x "$snes_cli" || ! -f .local/arduino-cli.yaml ]]; then
  printf 'Prepare the isolated Arduino environment described in docs/build.md.\n' >&2
  exit 1
fi

"$snes_cli" --config-file .local/arduino-cli.yaml --no-color compile \
  --fqbn 'UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=kbd,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash' \
  --build-property "compiler.cpp.extra_flags=-DSNES_HID_TEST_INPUT=$snes_test_input" \
  --build-path "$snes_project_dir/.local/build/$snes_mode" \
  firmware/snes_controller
