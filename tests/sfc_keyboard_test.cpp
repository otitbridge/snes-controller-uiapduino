// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setup();
void loop();

namespace {
uint16_t parallel = 0xffff;
unsigned serialBit = 0, reads = 0, sets = 0, starts = 0, pins = 0;
bool clockHigh = true;
uint8_t staged[8] = {};
void check(bool ok, const char* message) {
  if (!ok) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}
void sample(unsigned buttons) {
  parallel = static_cast<uint16_t>(~buttons);
  reads = 0;
  const unsigned before = sets;
  loop();
  check(sets == before + 1 && reads == 16, "one serial frame and one state per loop");
  const uint8_t usages[12] = {0x1d,0x04,0x2a,0x28,0x52,0x51,0x50,0x4f,0x1b,0x16,0x14,0x1a};
  uint8_t wanted[8] = {};
  unsigned slot = 2;
  for (unsigned bit = 0; bit < 12 && slot < 8; ++bit)
    if (buttons & (1u << bit)) wanted[slot++] = usages[bit];
  check(memcmp(staged, wanted, 8) == 0, "serial inputs reach the expected 8-byte keyboard state");
}
}

void digitalWrite(uint32_t pin, uint32_t value) {
  check(pin == 7 || pin == 8, "only CLOCK/LATCH are driven");
  if (pin == 8 && value == HIGH) serialBit = 0;
  if (pin == 7) {
    if (!clockHigh && value == HIGH) ++serialBit;
    clockHigh = value == HIGH;
  }
}
int digitalRead(uint32_t pin) {
  check(pin == 9 && serialBit < 16, "read DATA 9 before each clock advance");
  ++reads;
  return parallel & (1u << serialBit) ? HIGH : LOW;
}
void pinMode(uint32_t pin, uint32_t mode) {
  check((pin == 7 || pin == 8) ? mode == OUTPUT : pin == 9 && mode == INPUT_PULLUP,
        "SFC pin modes");
  ++pins;
}
void delay(unsigned long ms) { check(ms == 20 || ms == 10, "settling/poll interval"); }
void delayMicroseconds(unsigned us) { check(us == 12 || us == 6, "SFC requested delay"); }
extern "C" void uiapusb_begin() {
  const uint8_t neutral[8] = {};
  check(sets == 1 && memcmp(staged, neutral, 8) == 0, "neutral before USB starts");
  ++starts;
}
extern "C" void uiapkbd_keyboard_set(uint8_t modifier, uint8_t k0, uint8_t k1,
                                      uint8_t k2, uint8_t k3, uint8_t k4, uint8_t k5) {
  const uint8_t value[8] = {modifier,0,k0,k1,k2,k3,k4,k5};
  memcpy(staged, value, 8);
  ++sets;
}

int main() {
  setup();
  check(starts == 1 && pins == 3, "integrated sketch initializes SFC and USB");
  for (unsigned buttons = 0; buttons < 4096; ++buttons) sample(buttons);
  sample(0x83); sample(0x83); sample(0x82); sample(0);
  sample(0xd8b); sample(0xd83); sample(0);
  puts("PASS: actual SFC keyboard sketch; all 4096 serial inputs, holds, partial release and six-key recovery reach USB setter");
}
