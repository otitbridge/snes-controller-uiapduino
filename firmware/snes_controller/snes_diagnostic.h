// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#pragma once

#include "snes_reader.h"

namespace SnesDiagnostic {

// Fixed 8-byte vendor HID payload, report ID 0 (not part of these bytes).
struct Frame { uint8_t bytes[8]; };
static_assert(sizeof(Frame) == 8, "Diagnostic frame must fit one USB packet");

inline Frame makeFrame(SnesReader::Sample sample, uint8_t sequence) {
  return {{'S', 'F', 1, sequence,
           static_cast<uint8_t>(sample.raw), static_cast<uint8_t>(sample.raw >> 8),
           static_cast<uint8_t>(sample.pressed), static_cast<uint8_t>(sample.pressed >> 8)}};
}

void begin();
// If busy, leave the core buffer alone and retry with a fresh sample next loop.
// No transfer completion, host receipt, or short-pulse retention is implied.
void stageSample(SnesReader::Sample sample);

}  // namespace SnesDiagnostic
