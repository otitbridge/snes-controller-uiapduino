// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>
#include "board_config.h"
#include "snes_diagnostic.h"

#if SNES_SFC_DIAGNOSTIC
#include <uiapusb.h>
#if !defined(UIAP_COMPOSITE_HID) || !defined(UIAP_WEBHID) || defined(UIAP_WEBHID_ONLY) || \
    defined(UIAP_NO_USB) || defined(UIAP_USB_TERMINAL)
#error "SFC diagnostics require Keyboard+Mouse+WebHID (usb=kbdweb)."
#endif

namespace SnesDiagnostic {
namespace { uint8_t sequence = 0; }

void begin() {
  uiapkbd_keyboard_set(0, 0, 0, 0, 0, 0, 0);
  uiapusb_begin();
}

void stageSample(SnesReader::Sample sample) {
  // This is the only producer. ISR clears pending after reading the buffer.
  // Never enter send() while busy: its upstream timeout can overwrite pending data.
  if (uiapwebhid_tx_busy()) return;
  const Frame frame = makeFrame(sample, sequence++);
  uiapwebhid_send(frame.bytes, sizeof(frame.bytes));
}
}  // namespace SnesDiagnostic
#endif
