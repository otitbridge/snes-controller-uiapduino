// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#pragma once
#include <stdint.h>

// Test double for the two upstream APIs used by usb_backend.cpp. Real headers
// and implementations are checked separately by the MCU firmware build.
extern "C" {
void uiapusb_begin(void);
void uiapkbd_keyboard_set(uint8_t modifier,
                          uint8_t k0, uint8_t k1, uint8_t k2,
                          uint8_t k3, uint8_t k4, uint8_t k5);
}
