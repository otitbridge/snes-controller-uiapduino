// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include "usb_backend.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

static_assert(std::is_same<decltype(UsbBackend::stageButtons(0)), void>::value,
              "Staging must not claim transfer success");

namespace {
uint8_t staged[8] = {};
unsigned setCalls = 0;
unsigned beginCalls = 0;

void check(bool condition, const char* message) {
  if (!condition) {
    fprintf(stderr, "FAIL: %s\n", message);
    exit(EXIT_FAILURE);
  }
}

void expect(const uint8_t (&wanted)[8]) {
  check(memcmp(staged, wanted, sizeof(staged)) == 0, "exact 8-byte staged state");
}
}  // namespace

extern "C" void uiapusb_begin(void) {
  const uint8_t neutral[8] = {};
  check(setCalls > 0, "a state is staged before USB starts");
  expect(neutral);
  ++beginCalls;
}

extern "C" void uiapkbd_keyboard_set(uint8_t modifier,
                                      uint8_t k0, uint8_t k1, uint8_t k2,
                                      uint8_t k3, uint8_t k4, uint8_t k5) {
  const uint8_t report[] = {modifier, 0, k0, k1, k2, k3, k4, k5};
  memcpy(staged, report, sizeof(staged));
  ++setCalls;
}

int main() {
  UsbBackend::begin();
  check(beginCalls == 1 && setCalls == 1, "one initial neutral before begin");
  puts("PASS: initial neutral staged before USB begins");

  const uint8_t neutral[8] = {};
  const uint8_t z[8] = {0, 0, 0x1D, 0, 0, 0, 0, 0};
  UsbBackend::stageButtons(0x0001);
  expect(z);
  const unsigned beforeHold = setCalls;
  UsbBackend::stageButtons(0x0001);
  expect(z);
  check(setCalls == beforeHold + 1, "hold stages one complete current state");
  UsbBackend::stageButtons(0);
  expect(neutral);
  puts("PASS: manual B/Z press, hold, release; no extra release between holds");

  const uint8_t chord[8] = {0, 0, 0x1D, 0x04, 0x4F, 0, 0, 0};
  const uint8_t remaining[8] = {0, 0, 0x04, 0x4F, 0, 0, 0, 0};
  UsbBackend::stageButtons(0x0083);
  expect(chord);
  UsbBackend::stageButtons(0x0082);
  expect(remaining);
  puts("PASS: current-state mapping and partial release reach the setter");

  const uint8_t six[8] = {0, 0, 0x1D, 0x04, 0x4F, 0x1B, 0x14, 0x1A};
  const uint8_t seven[8] = {0, 0, 0x1D, 0x04, 0x28, 0x4F, 0x1B, 0x14};
  UsbBackend::stageButtons(0x0D8B);
  expect(seven);
  UsbBackend::stageButtons(0x0D83);
  expect(six);
  UsbBackend::stageButtons(0xF000);
  expect(neutral);
  puts("PASS: six-key selection and recovery; no ASCII or Report ID added");
  puts("PASS: USB backend API contract only; no real USB transfer observed");
}
