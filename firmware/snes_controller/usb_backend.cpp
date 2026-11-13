// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include "usb_backend.h"

#ifndef SNES_SFC_DIAGNOSTIC
#define SNES_SFC_DIAGNOSTIC 0
#endif
#if !SNES_SFC_DIAGNOSTIC

#include <uiapusb.h>

#include "snes_hid_report.h"

#if SNES_SFC_KEYBOARD && !defined(UIAP_SNES_COHERENT_KEYBOARD)
#error "SFC keyboard requires the verified project-local coherent keyboard core."
#endif

#if !defined(UIAP_COMPOSITE_HID) || defined(UIAP_WEBHID_ONLY) || \
    defined(UIAP_NO_USB) || defined(UIAP_USB_TERMINAL)
#error "Select the UIAPduino Keyboard+Mouse USB mode (usb=kbd)."
#endif

namespace UsbBackend {

void begin() {
  stageButtons(0);
  uiapusb_begin();
}

void stageButtons(uint16_t buttons, bool selectBackspace) {
  const auto desired = SnesHid::makeReport(buttons, selectBackspace);
  uiapkbd_keyboard_set(desired.modifiers,
                       desired.keys[0], desired.keys[1], desired.keys[2],
                       desired.keys[3], desired.keys[4], desired.keys[5]);
}

}  // namespace UsbBackend
#endif
