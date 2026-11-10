// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#pragma once

// The default firmware never reads a GPIO or presses a key.
// Opt in at build time only after checking the board and test switch wiring.
#ifndef SNES_HID_TEST_INPUT
#define SNES_HID_TEST_INPUT 0
#endif
#ifndef SNES_SFC_DIAGNOSTIC
#define SNES_SFC_DIAGNOSTIC 0
#endif
#ifndef SNES_SFC_KEYBOARD
#define SNES_SFC_KEYBOARD 0
#endif

#if SNES_HID_TEST_INPUT != 0 && SNES_HID_TEST_INPUT != 1
#error "SNES_HID_TEST_INPUT must be 0 (neutral) or 1 (manual switch)."
#endif
#if SNES_SFC_DIAGNOSTIC != 0 && SNES_SFC_DIAGNOSTIC != 1
#error "SNES_SFC_DIAGNOSTIC must be 0 or 1."
#endif
#if SNES_SFC_KEYBOARD != 0 && SNES_SFC_KEYBOARD != 1
#error "SNES_SFC_KEYBOARD must be 0 or 1."
#endif
#if SNES_SFC_KEYBOARD && SNES_SFC_DIAGNOSTIC
#error "Select either SFC keyboard or SFC diagnostic mode."
#endif
#if SNES_HID_TEST_INPUT && (SNES_SFC_DIAGNOSTIC || SNES_SFC_KEYBOARD)
#error "Remove the PC5/GND jumper before using PC5 as the SFC CLOCK output."
#endif

namespace BoardConfig {
constexpr unsigned long kPollIntervalMs = 10;

#if SNES_HID_TEST_INPUT
// UIAPduino V1.4 / tarosay core 1.2.15: PC5 = GPIO_PIN_7.
// External momentary switch to GND, INPUT_PULLUP; active LOW maps to B -> Z.
constexpr uint8_t kTestInputPin = GPIO_PIN_7;
#endif
#if SNES_SFC_DIAGNOSTIC || SNES_SFC_KEYBOARD
// Board silkscreen 7/8/9; these are not IC package pin numbers.
constexpr uint8_t kClockPin = GPIO_PIN_7;  // PC5
constexpr uint8_t kLatchPin = GPIO_PIN_8;  // PC6
constexpr uint8_t kDataPin = GPIO_PIN_9;   // PC7
#endif
}  // namespace BoardConfig
