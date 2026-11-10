// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
// Adapted from src/keyboard_state.h in snes-controller-m5stick-s3,
// commit 21b7ba91c6428b25d7b7ab116052443a9aced965. See LICENSE.
#pragma once

#include <stdint.h>

namespace SnesHid {

// Bit masks in SFC/SNES serial order. Internal state uses 1 = pressed.
enum Button : uint16_t {
  kB      = 1U << 0,
  kY      = 1U << 1,
  kSelect = 1U << 2,
  kStart  = 1U << 3,
  kUp     = 1U << 4,
  kDown   = 1U << 5,
  kLeft   = 1U << 6,
  kRight  = 1U << 7,
  kA      = 1U << 8,
  kX      = 1U << 9,
  kL      = 1U << 10,
  kR      = 1U << 11,
};

constexpr uint8_t kButtonCount = 12;
constexpr uint16_t kButtonMask = 0x0FFF;
constexpr uint8_t kMaxKeys = 6;

// Eight-byte keyboard payload. Report ID, USB staging and transfer are separate.
struct Report {
  uint8_t modifiers = 0;
  uint8_t reserved = 0;
  uint8_t keys[kMaxKeys] = {};
};
static_assert(sizeof(Report) == 8, "Keyboard report must be exactly 8 bytes");

// Recompute from the complete current state; no retained state or USB calls.
// If more than six buttons are held, the first six in bit order take priority.
inline Report makeReport(uint16_t buttons) {
  // Raw HID usages: Z, A, Backspace, Enter, Up, Down, Left, Right, X, S, Q, W.
  constexpr uint8_t usages[] = {
      0x1D, 0x04, 0x2A, 0x28, 0x52, 0x51,
      0x50, 0x4F, 0x1B, 0x16, 0x14, 0x1A};
  static_assert(sizeof(usages) == kButtonCount, "Every button needs a usage");

  buttons &= kButtonMask;
  Report report;
  uint8_t count = 0;
  for (uint8_t bit = 0; bit < kButtonCount; ++bit) {
    if (buttons & (1U << bit)) {
      report.keys[count++] = usages[bit];
      if (count == kMaxKeys) break;
    }
  }
  return report;
}

}  // namespace SnesHid
