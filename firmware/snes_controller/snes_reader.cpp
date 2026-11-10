// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>
#include "board_config.h"
#include "snes_reader.h"

#if SNES_SFC_DIAGNOSTIC
namespace SnesReader {
namespace {
struct ArduinoIO {
  void clock(bool high) { digitalWrite(BoardConfig::kClockPin, high ? HIGH : LOW); }
  void latch(bool high) { digitalWrite(BoardConfig::kLatchPin, high ? HIGH : LOW); }
  bool dataHigh() { return digitalRead(BoardConfig::kDataPin) == HIGH; }
  void waitUs(unsigned us) { delayMicroseconds(us); }
};
}  // namespace

void begin() {
  // Preload output levels before enabling output drivers.
  digitalWrite(BoardConfig::kClockPin, HIGH);
  digitalWrite(BoardConfig::kLatchPin, LOW);
  pinMode(BoardConfig::kClockPin, OUTPUT);
  pinMode(BoardConfig::kLatchPin, OUTPUT);
  pinMode(BoardConfig::kDataPin, INPUT_PULLUP);
  delay(20);  // Same initial settling allowance as the reference firmware.
}

Sample read() {
  ArduinoIO io;
  return readFrame(io);
}
}  // namespace SnesReader
#endif
