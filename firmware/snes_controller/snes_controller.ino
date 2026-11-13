// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>

#include "board_config.h"
#include "snes_hid_report.h"
#include "usb_backend.h"
#include "snes_reader.h"
#include "snes_diagnostic.h"

#if SNES_SFC_KEYBOARD
namespace {
bool selectBackspace = false;
bool suppressBootSelect = false;
}
#endif

void setup() {
#if SNES_SFC_DIAGNOSTIC || SNES_SFC_KEYBOARD
  SnesReader::begin();
#endif
#if SNES_SFC_KEYBOARD
  // Reader::begin() already allows 20 ms for power settling. Require SELECT
  // in three consecutive frames, 10 ms apart, before USB begins.
  uint16_t held = SnesHid::kSelect;
  uint16_t seen = 0;
  for (uint8_t i = 0; i < 3; ++i) {
    const uint16_t buttons = SnesReader::read().pressed;
    held &= buttons;
    seen |= buttons;
    if (i < 2) delay(BoardConfig::kPollIntervalMs);
  }
  selectBackspace = (held & SnesHid::kSelect) != 0;
  // Even an unstable startup SELECT must be released before sending a key.
  suppressBootSelect = (seen & SnesHid::kSelect) != 0;
#endif
#if SNES_SFC_DIAGNOSTIC
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
#if SNES_SFC_KEYBOARD
  desiredButtons = SnesReader::read().pressed;
  if (suppressBootSelect) {
    if (!(desiredButtons & SnesHid::kSelect)) suppressBootSelect = false;
    desiredButtons &= ~SnesHid::kSelect;
  }
#endif
#if SNES_HID_TEST_INPUT
  if (digitalRead(BoardConfig::kTestInputPin) == LOW) {
    desiredButtons = SnesHid::kB;
  }
#endif
#if SNES_SFC_KEYBOARD
  UsbBackend::stageButtons(desiredButtons, selectBackspace);
#else
  UsbBackend::stageButtons(desiredButtons);
#endif
#endif
  // delay() and the optional SFC reader leave USB interrupts enabled.
  delay(BoardConfig::kPollIntervalMs);
}
