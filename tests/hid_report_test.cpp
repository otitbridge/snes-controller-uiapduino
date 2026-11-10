// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include "snes_hid_report.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <initializer_list>
#include <type_traits>

// The Arduino library declares a global object named Keyboard. This stand-in
// makes an accidental reintroduction of namespace Keyboard a compile error.
struct KeyboardStandIn {};
KeyboardStandIn Keyboard;

static_assert(std::is_standard_layout<SnesHid::Report>::value, "Wire layout");
static_assert(std::is_trivially_copyable<SnesHid::Report>::value, "Plain payload");
static_assert(offsetof(SnesHid::Report, modifiers) == 0, "Modifier byte offset");
static_assert(offsetof(SnesHid::Report, reserved) == 1, "Reserved byte offset");
static_assert(offsetof(SnesHid::Report, keys) == 2, "Key array offset");

namespace {

// Independent fixtures from handoff section 6.1. Expected masks and usages do
// not come from the production header's mapping table or button constants.
struct ButtonCase {
  uint16_t mask;
  uint8_t usage;
  SnesHid::Button definition;
};
constexpr ButtonCase kCases[] = {
    {0x0001, 0x1D, SnesHid::kB},
    {0x0002, 0x04, SnesHid::kY},
    {0x0004, 0x2A, SnesHid::kSelect},
    {0x0008, 0x28, SnesHid::kStart},
    {0x0010, 0x52, SnesHid::kUp},
    {0x0020, 0x51, SnesHid::kDown},
    {0x0040, 0x50, SnesHid::kLeft},
    {0x0080, 0x4F, SnesHid::kRight},
    {0x0100, 0x1B, SnesHid::kA},
    {0x0200, 0x16, SnesHid::kX},
    {0x0400, 0x14, SnesHid::kL},
    {0x0800, 0x1A, SnesHid::kR},
};

uint16_t currentMask = 0;

// Keep checks active even with NDEBUG, as used by the target core.
#define CHECK(condition)                                                     \
  do {                                                                       \
    if (!(condition)) {                                                      \
      fprintf(stderr, "FAIL line %d mask=0x%04X: %s\n", __LINE__,              \
              static_cast<unsigned>(currentMask), #condition);               \
      exit(EXIT_FAILURE);                                                    \
    }                                                                        \
  } while (false)

bool equal(const SnesHid::Report& a, const SnesHid::Report& b) {
  return memcmp(&a, &b, sizeof(a)) == 0;
}

void expect(uint16_t mask, std::initializer_list<uint8_t> keys) {
  currentMask = mask;
  const auto report = SnesHid::makeReport(mask);
  CHECK(report.modifiers == 0);
  CHECK(report.reserved == 0);
  size_t index = 0;
  for (const auto key : keys) {
    CHECK(index < 6);
    CHECK(report.keys[index++] == key);
  }
  while (index < 6) CHECK(report.keys[index++] == 0);
}

void checkProperties(uint16_t mask) {
  currentMask = mask;
  const auto report = SnesHid::makeReport(mask);
  CHECK(report.modifiers == 0);
  CHECK(report.reserved == 0);

  unsigned heldCount = 0;
  for (uint16_t remaining = mask; remaining; remaining &= remaining - 1) {
    ++heldCount;
  }
  const unsigned expectedCount = heldCount < 6 ? heldCount : 6;
  int previousRank = -1;
  for (unsigned slot = 0; slot < 6; ++slot) {
    const uint8_t usage = report.keys[slot];
    if (slot >= expectedCount) {
      CHECK(usage == 0);
      continue;
    }
    int rank = -1;
    for (unsigned i = 0; i < 12; ++i) {
      if (kCases[i].usage == usage) rank = static_cast<int>(i);
    }
    CHECK(rank >= 0);
    CHECK(mask & kCases[rank].mask);
    CHECK(rank > previousRank);  // Bit-order priority and no duplicate usages.
    previousRank = rank;
  }

  // Verify inclusion/exclusion for every button, independent of output slots.
  unsigned pressedRank = 0;
  for (const auto& button : kCases) {
    bool present = false;
    for (const auto usage : report.keys) present |= usage == button.usage;
    const bool pressed = (mask & button.mask) != 0;
    CHECK(present == (pressed && pressedRank < 6));
    if (pressed) ++pressedRank;
  }
}

void testSingleButtons() {
  CHECK(SnesHid::kButtonCount == 12);
  CHECK(SnesHid::kButtonMask == 0x0FFF);
  CHECK(SnesHid::kMaxKeys == 6);
  expect(0, {});
  for (const auto& button : kCases) {
    currentMask = button.mask;
    CHECK(static_cast<uint16_t>(button.definition) == button.mask);
    expect(button.mask, {button.usage});
  }
  expect(0x0FFF, {0x1D, 0x04, 0x2A, 0x28, 0x52, 0x51});
  puts("PASS L01: neutral, 12 single buttons, and all-buttons golden report");
}

void testEveryCombination() {
  for (uint16_t mask = 0; mask < 4096; ++mask) checkProperties(mask);
  puts("PASS L02: 4096 combinations (membership, priority, padding, uniqueness)");
}

void testUnusedBits() {
  for (uint32_t raw = 0; raw < 65536; ++raw) {
    currentMask = static_cast<uint16_t>(raw);
    CHECK(equal(SnesHid::makeReport(currentMask),
                SnesHid::makeReport(static_cast<uint16_t>(raw & 0x0FFF))));
  }
  puts("PASS: all 65536 uint16_t inputs ignore the unused upper four bits");
}

void testHoldAndHistory() {
  for (const uint16_t mask : {0x0001, 0x0083, 0x0FFF}) {
    currentMask = mask;
    const auto held = SnesHid::makeReport(mask);
    for (unsigned poll = 0; poll < 3000; ++poll) {
      CHECK(equal(held, SnesHid::makeReport(mask)));
    }
  }
  for (uint16_t mask = 0; mask < 4096; ++mask) {
    currentMask = mask;
    const auto first = SnesHid::makeReport(mask);
    checkProperties(static_cast<uint16_t>(mask ^ 0x0FFF));
    currentMask = mask;
    CHECK(equal(first, SnesHid::makeReport(mask)));
  }
  puts("PASS: repeated holds and results independent of intervening inputs");
}

void testPartialReleaseAndOverflow() {
  // Right + Y + B -> release only B -> right -> full release.
  expect(0x0083, {0x1D, 0x04, 0x4F});
  expect(0x0082, {0x04, 0x4F});
  expect(0x0080, {0x4F});
  expect(0, {});

  // Handoff D05: add START to six held buttons; R loses its output slot.
  expect(0x0D83, {0x1D, 0x04, 0x4F, 0x1B, 0x14, 0x1A});
  expect(0x0D8B, {0x1D, 0x04, 0x28, 0x4F, 0x1B, 0x14});
  expect(0x0D83, {0x1D, 0x04, 0x4F, 0x1B, 0x14, 0x1A});
  expect(0x007F, {0x1D, 0x04, 0x2A, 0x28, 0x52, 0x51});
  expect(0x007E, {0x04, 0x2A, 0x28, 0x52, 0x51, 0x50});
  expect(0x0F00, {0x1B, 0x16, 0x14, 0x1A});
  expect(0, {});

  unsigned releaseCases = 0;
  for (uint16_t mask = 0; mask < 4096; ++mask) {
    for (const auto& button : kCases) {
      if (!(mask & button.mask)) continue;
      checkProperties(mask);
      checkProperties(static_cast<uint16_t>(mask & ~button.mask));
      ++releaseCases;
    }
  }
  CHECK(releaseCases == 24576);
  puts("PASS L03: overflow recovery and 24576 single-button release transitions");
}

}  // namespace

int main() {
  testSingleButtons();
  testEveryCombination();
  testUnusedBits();
  testHoldAndHistory();
  testPartialReleaseAndOverflow();
  puts("PASS: all host report tests (no USB or MCU headers)");
}
