// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include "usb_backend.h"

#include <uiapusb.h>

#include "snes_hid_report.h"

#if !defined(UIAP_COMPOSITE_HID) || defined(UIAP_WEBHID_ONLY) || \
    defined(UIAP_NO_USB) || defined(UIAP_USB_TERMINAL)
#error "Select the UIAPduino Keyboard+Mouse USB mode (usb=kbd)."
#endif

namespace UsbBackend {

void begin() {
  stageButtons(0);
  uiapusb_begin();
}

void stageButtons(uint16_t buttons) {
  const auto desired = SnesHid::makeReport(buttons);
  uiapkbd_keyboard_set(desired.modifiers,
                       desired.keys[0], desired.keys[1], desired.keys[2],
                       desired.keys[3], desired.keys[4], desired.keys[5]);
}

}  // namespace UsbBackend
