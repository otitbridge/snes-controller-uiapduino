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
