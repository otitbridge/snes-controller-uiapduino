// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>
#include "snes_reader.h"
#include "snes_diagnostic.h"
#include "snes_hid_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {
void check(bool ok, const char* message) {
  if (!ok) { fprintf(stderr, "FAIL: %s\n", message); exit(EXIT_FAILURE); }
}

// Model a parallel-load shift register. DATA changes on CLOCK rising edges,
// never because it was read. This catches sampling after advancing a bit.
struct Controller {
  uint16_t parallel = 0xFFFF;
  bool clockLevel = true, latchLevel = false, loaded = false;
  unsigned bit = 0, reads = 0, rises = 0, lows = 0;
  unsigned time = 0, latchStarted = 0, latchEnded = 0, clockLowAt = 0;

  void clock(bool high) {
    if (!high) { check(clockLevel, "clock transitions HIGH to LOW"); ++lows; clockLowAt = time; }
    if (high && !clockLevel) {
      check(time - clockLowAt == 6, "requested CLOCK LOW interval is 6 us");
      ++rises;
      if (loaded && !latchLevel) ++bit;
    }
    clockLevel = high;
  }
  void latch(bool high) {
    check(high != latchLevel, "latch changes level");
    if (high) { check(clockLevel, "CLOCK HIGH before LATCH"); latchStarted = time; bit = 0; loaded = true; }
    else { check(time - latchStarted == 12, "requested latch pulse is 12 us"); latchEnded = time; }
    latchLevel = high;
  }
  void waitUs(unsigned us) { time += us; }
  bool dataHigh() {
    check(loaded && !latchLevel && clockLevel, "read DATA after latching with CLOCK HIGH");
    check(bit == reads && bit < 16, "read first bit before advancing, including all four tail bits");
    check(time - latchEnded == 6 + 12 * reads, "requested sample times follow the reference");
    ++reads;
    return (parallel & (1U << bit)) != 0;
  }
  void finish() const {
    check(reads == 16 && rises == 16 && lows == 16, "sixteen DATA samples and clock cycles");
    check(clockLevel && !latchLevel && time == 210, "idle levels and requested frame time");
  }
};

Controller hardware;
unsigned writes = 0, modes = 0, settling = 0;
uint8_t modePins[3] = {}, modeValues[3] = {};
bool busy = false;
unsigned usbBegins = 0, sends = 0;
unsigned keyboardSets = 0;
uint8_t payload[8] = {};
}  // namespace

void digitalWrite(uint32_t pin, uint32_t value) {
  ++writes;
  if (pin == 7) hardware.clock(value == HIGH);
  else if (pin == 8) {
    // begin() preloads the already-LOW latch without a pulse.
    if (value == LOW && !hardware.latchLevel && !hardware.loaded) return;
    hardware.latch(value == HIGH);
  } else check(false, "only CLOCK and LATCH are driven");
}
int digitalRead(uint32_t pin) {
  check(pin == 9, "DATA uses board 9 / PC7");
  return hardware.dataHigh() ? HIGH : LOW;
}
void pinMode(uint32_t pin, uint32_t mode) {
  check(modes < 3, "three SFC pins configured");
  modePins[modes] = static_cast<uint8_t>(pin);
  modeValues[modes++] = static_cast<uint8_t>(mode);
}
void delay(unsigned long ms) { settling += static_cast<unsigned>(ms); }
void delayMicroseconds(unsigned int us) { hardware.waitUs(us); }
extern "C" void uiapusb_begin() { check(keyboardSets == 1, "neutral before USB begins"); ++usbBegins; }
extern "C" void uiapkbd_keyboard_set(uint8_t modifier, uint8_t k0, uint8_t k1,
                                       uint8_t k2, uint8_t k3, uint8_t k4, uint8_t k5) {
  check((modifier | k0 | k1 | k2 | k3 | k4 | k5) == 0, "diagnostics stage neutral only");
  ++keyboardSets;
}
extern "C" uint8_t uiapwebhid_tx_busy() { return busy; }
extern "C" void uiapwebhid_send(const uint8_t* buf, uint8_t len) {
  check(!busy && len == 8, "only publish one complete frame to an idle buffer");
  memcpy(payload, buf, 8); busy = true; ++sends;
}

int main() {
  for (unsigned raw = 0; raw <= 0xFFFF; ++raw) {
    Controller io;
    io.parallel = static_cast<uint16_t>(raw);
    const auto sample = SnesReader::readFrame(io);
    io.finish();
    check(sample.raw == raw, "preserve all sixteen sampled levels in serial order");
    check(sample.pressed == ((~raw) & 0x0FFF), "active LOW lower twelve bits only");
    const auto frame = SnesDiagnostic::makeFrame(sample, 0xA7);
    check(frame.bytes[0] == 'S' && frame.bytes[1] == 'F' && frame.bytes[2] == 1 && frame.bytes[3] == 0xA7,
          "diagnostic signature, version, sequence");
    check(static_cast<unsigned>(frame.bytes[4] | (frame.bytes[5] << 8)) == raw &&
          (frame.bytes[6] | (frame.bytes[7] << 8)) == sample.pressed,
          "raw and decoded fields are little endian");
  }
  puts("PASS: all 65536 serial frames, sample order, active LOW, tail masking and requested timing");

  SnesReader::begin();
  check(writes == 2 && modes == 3 && settling == 20, "preload outputs, configure pins, settle 20 ms");
  check(modePins[0] == 7 && modePins[1] == 8 && modePins[2] == 9 &&
        modeValues[0] == OUTPUT && modeValues[1] == OUTPUT && modeValues[2] == INPUT_PULLUP,
        "Arduino adapter configures CLOCK 7, LATCH 8, DATA 9");
  hardware.parallel = 0xFFFE;
  const auto b = SnesReader::read();
  hardware.finish();
  check(b.raw == 0xFFFE && b.pressed == SnesHid::kB, "real adapter reads first serial bit as B");
  puts("PASS: Arduino adapter pin roles and B-bit decoding");

  SnesDiagnostic::begin();
  check(usbBegins == 1, "diagnostic USB initialization");
  SnesDiagnostic::stageSample(b);
  check(sends == 1 && payload[3] == 0 && payload[4] == 0xFE && payload[6] == 1, "first staged diagnostic frame");
  const uint8_t previous[8] = {'S','F',1,0,0xFE,0xFF,1,0};
  SnesDiagnostic::stageSample({0xFFFF, 0});
  check(sends == 1 && memcmp(payload, previous, 8) == 0, "busy buffer is never overwritten");
  for (unsigned n = 1; n <= 256; ++n) {
    busy = false;
    SnesDiagnostic::stageSample({0xFFFF, 0});
    check(payload[3] == static_cast<uint8_t>(n) && payload[4] == 0xFF && payload[6] == 0,
          "fresh sample after busy; staged sequence wraps at 256");
  }
  puts("PASS: diagnostic busy-buffer preservation and bounded sequence; no USB transfer claimed");
}
