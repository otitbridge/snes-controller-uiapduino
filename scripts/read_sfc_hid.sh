#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 otitbridge
set -euo pipefail
snes_project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
cd "$snes_project_dir"
mkdir -p .local/bin
clang -std=c11 -Wall -Wextra -Werror scripts/read_sfc_hid.c \
  -framework IOKit -framework CoreFoundation -o .local/bin/read_sfc_hid
exec .local/bin/read_sfc_hid "${1:-180}"
