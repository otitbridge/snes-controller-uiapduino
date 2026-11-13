// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#pragma once

#include <stdint.h>

namespace UsbBackend {

// Stage neutral before enabling USB. Call once from setup().
void begin();

// Replace the core's latest keyboard state. This returns no transfer status:
// the core setter does not acknowledge polling, host receipt, or application use.
// Call from loop() only. SFC keyboard builds require the project-local core patch
// that publishes complete reports. Other modes retain upstream core 1.2.15.
void stageButtons(uint16_t buttons, bool selectBackspace = false);

}  // namespace UsbBackend
