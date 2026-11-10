// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#pragma once

// The default firmware never reads a GPIO or presses a key.
// Opt in at build time only after checking the board and test switch wiring.
#ifndef SNES_HID_TEST_INPUT
#define SNES_HID_TEST_INPUT 0
#endif

#if SNES_HID_TEST_INPUT != 0 && SNES_HID_TEST_INPUT != 1
#error "SNES_HID_TEST_INPUT must be 0 (neutral) or 1 (manual switch)."
#endif

namespace BoardConfig {
constexpr unsigned long kPollIntervalMs = 10;

#if SNES_HID_TEST_INPUT
// UIAPduino V1.4 / tarosay core 1.2.15: PC5 = GPIO_PIN_7.
// External momentary switch to GND, INPUT_PULLUP; active LOW maps to B -> Z.
constexpr uint8_t kTestInputPin = GPIO_PIN_7;
#endif
}  // namespace BoardConfig
