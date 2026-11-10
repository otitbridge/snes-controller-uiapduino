// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
// The generated include contains the actual core storage, writer and reader.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t previous[8], desired[8], observed[8];
static unsigned interrupts, transfers;
static int injecting;
static void test_interrupt(void);
static void usb_send_data(uint8_t* data, uint32_t length,
                          uint32_t function, uint32_t token) {
    if (length != 8 || function != 0 || token != 0x4b) exit(2);
    memcpy(observed, data, 8);
    ++transfers;
}

#include "tested_core_keyboard.h"

static void test_interrupt(void) {
    if (!injecting) return;
    ++interrupts;
    read_keyboard();
    if (memcmp(observed, previous, 8) != 0 && memcmp(observed, desired, 8) != 0) {
        fputs("Mixed keyboard snapshot at an injected interrupt\n", stderr);
        exit(1);
    }
}

static void change(int clear) {
    injecting = 1;
    if (clear) uiapkbd_keyboard_clear();
    else uiapkbd_keyboard_set(desired[0], desired[2], desired[3], desired[4],
                              desired[5], desired[6], desired[7]);
    injecting = 0;
    read_keyboard();
    if (memcmp(observed, desired, 8) != 0) exit(3);
    memcpy(previous, desired, 8);
}

int main(void) {
    uint32_t random = 0x173849ab;
    read_keyboard();
    if (memcmp(observed, previous, 8) != 0) return 4;
    for (unsigned iteration = 0; iteration < 4096; ++iteration) {
        for (unsigned byte = 0; byte < 8; ++byte) {
            random = random * 1664525u + 1013904223u;
            desired[byte] = (uint8_t)(random >> 24);
        }
        desired[1] = 0;
        change(0);
        // A repeated hold and clear also exercise repeated publication/reuse.
        change(0);
        memset(desired, 0, 8);
        change(1);
    }
    if (interrupts < 4096 * 3 * 8 || transfers <= interrupts) return 5;
    printf("PASS: actual core writer/reader; %u injected interruptions; "
           "only complete old/new states, repeated holds and clear\n", interrupts);
    return 0;
}
