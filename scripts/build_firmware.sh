#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
set -euo pipefail

snes_project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
cd "$snes_project_dir"
snes_mode="${1:-neutral}"
snes_diagnostic=0
snes_keyboard=0
snes_usb=kbd
snes_config=.local/arduino-cli.yaml
case "$snes_mode" in
  neutral) snes_test_input=0 ;;
  manual) snes_test_input=1 ;;
  sfc-diagnostic) snes_test_input=0; snes_diagnostic=1; snes_usb=kbdweb ;;
  sfc-keyboard) snes_test_input=0; snes_keyboard=1 ;;
  *) printf 'Usage: bash scripts/build_firmware.sh [neutral|manual|sfc-diagnostic|sfc-keyboard]\n' >&2; exit 2 ;;
esac

snes_cli=.local/bin/arduino-cli
if [[ ! -x "$snes_cli" || ! -f .local/arduino-cli.yaml ]]; then
  printf 'Prepare the isolated Arduino environment described in docs/build.md.\n' >&2
  exit 1
fi

if [[ "$snes_keyboard" == 1 ]]; then
  python3 scripts/prepare_sfc_core.py
  snes_config=.local/sfc-arduino-cli.yaml
fi

"$snes_cli" --config-file "$snes_config" --no-color compile \
  --fqbn "UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=$snes_usb,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash" \
  --build-property "compiler.cpp.extra_flags=-DSNES_HID_TEST_INPUT=$snes_test_input -DSNES_SFC_DIAGNOSTIC=$snes_diagnostic -DSNES_SFC_KEYBOARD=$snes_keyboard" \
  --build-path "$snes_project_dir/.local/build/$snes_mode" \
  firmware/snes_controller
