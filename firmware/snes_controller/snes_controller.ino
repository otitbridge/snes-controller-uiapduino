// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>

#include "board_config.h"
#include "snes_hid_report.h"
#include "usb_backend.h"

void setup() {
#if SNES_HID_TEST_INPUT
  pinMode(BoardConfig::kTestInputPin, INPUT_PULLUP);
#endif
  UsbBackend::begin();
}

void loop() {
  uint16_t desiredButtons = 0;
#if SNES_HID_TEST_INPUT
  if (digitalRead(BoardConfig::kTestInputPin) == LOW) {
    desiredButtons = SnesHid::kB;
  }
#endif
  UsbBackend::stageButtons(desiredButtons);
  // delay() leaves USB interrupts enabled; no timer ISR or SFC reader is added.
  delay(BoardConfig::kPollIntervalMs);
}
