// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#pragma once
#include <stdint.h>

// Narrow test double. MCU builds use the installed core's real Arduino.h.
constexpr uint8_t GPIO_PIN_7 = 7;
constexpr uint8_t GPIO_PIN_8 = 8;
constexpr uint8_t GPIO_PIN_9 = 9;
constexpr uint8_t HIGH = 1;
constexpr uint8_t LOW = 0;
constexpr uint8_t OUTPUT = 1;
constexpr uint8_t INPUT_PULLUP = 2;
void digitalWrite(uint32_t pin, uint32_t value);
int digitalRead(uint32_t pin);
void pinMode(uint32_t pin, uint32_t mode);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
