// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
// Adapted from readController() in snes-controller-m5stick-s3,
// commit 21b7ba91c6428b25d7b7ab116052443a9aced965. See LICENSE.
#pragma once

#include <stdint.h>

namespace SnesReader {

struct Sample {
  uint16_t raw;      // Serial DATA levels, first sample in bit 0; 1 = HIGH.
  uint16_t pressed;  // Active LOW decoded into the lower 12 button bits.
};

// IO supplies clock(bool), latch(bool), dataHigh(), waitUs(unsigned).
// Keep USB interrupts enabled throughout. Delays are requested minima;
// interrupt service may lengthen the actual waveform.
template <typename IO>
Sample readFrame(IO& io) {
  io.clock(true);
  io.latch(true);
  io.waitUs(12);
  io.latch(false);
  io.waitUs(6);
  uint16_t raw = 0;
  for (uint8_t bit = 0; bit < 16; ++bit) {
    if (io.dataHigh()) raw |= static_cast<uint16_t>(1U << bit);
    io.clock(false);
    io.waitUs(6);
    io.clock(true);
    io.waitUs(6);
  }
  return {raw, static_cast<uint16_t>((~raw) & 0x0FFFU)};
}

// Arduino adapter; available only in explicit SFC diagnostic/keyboard builds.
void begin();
Sample read();

}  // namespace SnesReader
