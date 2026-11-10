#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
set -euo pipefail

snes_project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
cd "$snes_project_dir"
mkdir -p .local/host-tests

snes_cxx="${CXX:-c++}"
snes_flags=(-std=c++14 -Wall -Wextra -Werror -pedantic -DNDEBUG -O2)
snes_binary=.local/host-tests/hid_report_test
if [[ "${SNES_HOST_SANITIZERS:-0}" == 1 ]]; then
  snes_flags+=(-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
  snes_binary=.local/host-tests/hid_report_test_sanitized
fi

"$snes_cxx" "${snes_flags[@]}" -Ifirmware/snes_controller \
  tests/hid_report_test.cpp -o "$snes_binary"
"$snes_binary"

snes_usb_binary="${snes_binary%hid_report_test*}usb_backend_test"
if [[ "${SNES_HOST_SANITIZERS:-0}" == 1 ]]; then
  snes_usb_binary+=_sanitized
fi
"$snes_cxx" "${snes_flags[@]}" -DUIAP_COMPOSITE_HID \
  -Itests/fakes -Ifirmware/snes_controller \
  tests/usb_backend_test.cpp firmware/snes_controller/usb_backend.cpp \
  -o "$snes_usb_binary"
"$snes_usb_binary"

snes_reader_binary="${snes_binary%hid_report_test*}snes_reader_test"
if [[ "${SNES_HOST_SANITIZERS:-0}" == 1 ]]; then
  snes_reader_binary+=_sanitized
fi
"$snes_cxx" "${snes_flags[@]}" -DSNES_SFC_DIAGNOSTIC=1 -DUIAP_COMPOSITE_HID -DUIAP_WEBHID \
  -Itests/fakes -Ifirmware/snes_controller \
  tests/snes_reader_test.cpp firmware/snes_controller/snes_reader.cpp \
  firmware/snes_controller/snes_diagnostic.cpp -o "$snes_reader_binary"
"$snes_reader_binary"

snes_keyboard_binary="${snes_binary%hid_report_test*}sfc_keyboard_test"
if [[ "${SNES_HOST_SANITIZERS:-0}" == 1 ]]; then
  snes_keyboard_binary+=_sanitized
fi
"$snes_cxx" "${snes_flags[@]}" -DSNES_SFC_KEYBOARD=1 \
  -DUIAP_COMPOSITE_HID -DUIAP_SNES_COHERENT_KEYBOARD=1 \
  -Itests/fakes -Ifirmware/snes_controller \
  tests/sfc_keyboard_test.cpp firmware/snes_controller/snes_reader.cpp \
  firmware/snes_controller/usb_backend.cpp -x c++ firmware/snes_controller/snes_controller.ino \
  -o "$snes_keyboard_binary"
"$snes_keyboard_binary"
