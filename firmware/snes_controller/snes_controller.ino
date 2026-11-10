// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>

#include "board_config.h"
#include "snes_hid_report.h"
#include "usb_backend.h"
#include "snes_reader.h"
#include "snes_diagnostic.h"

void setup() {
#if SNES_SFC_DIAGNOSTIC
  SnesReader::begin();
  SnesDiagnostic::begin();
#else
#if SNES_HID_TEST_INPUT
  pinMode(BoardConfig::kTestInputPin, INPUT_PULLUP);
#endif
  UsbBackend::begin();
#endif
}

void loop() {
#if SNES_SFC_DIAGNOSTIC
  SnesDiagnostic::stageSample(SnesReader::read());
#else
  uint16_t desiredButtons = 0;
#if SNES_HID_TEST_INPUT
  if (digitalRead(BoardConfig::kTestInputPin) == LOW) {
    desiredButtons = SnesHid::kB;
  }
#endif
  UsbBackend::stageButtons(desiredButtons);
#endif
  // delay() and the optional SFC reader leave USB interrupts enabled.
  delay(BoardConfig::kPollIntervalMs);
}
