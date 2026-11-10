// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Only the two endpoint fields used by the extracted core callback/routing. */
struct usb_endpoint { unsigned max_len; uint8_t *opaque; unsigned count, toggle_in; };
struct rv003usb_internal { struct usb_endpoint eps[3]; };
struct usb_urb { uint16_t wRequestTypeLSBRequestMSB; uint32_t lValueLSBIndexMSB; uint16_t wLength; };
static void usb_send_data(const void *data, uint32_t length, uint32_t poly, uint32_t token)
    __attribute__((unused));
static uint8_t sent_report[8];
static void usb_send_data(const void *data, uint32_t length, uint32_t poly, uint32_t token)
{
    (void)poly; (void)token;
    if (length != 8) abort();
    memcpy(sent_report, data, 8);
}
#include "tested_core_get_report.h"

static void require(int condition, const char *message)
{
    if (!condition) { fprintf(stderr, "%s\n", message); exit(1); }
}

static void expect_report(struct usb_endpoint *e, int length, uint32_t wvi,
                          const uint8_t *expected, unsigned size)
{
    dispatch_report(e, 0x01a1, length, wvi);
    const unsigned limit = (unsigned)length < size ? (unsigned)length : size;
    require(e->opaque != NULL && e->max_len == limit, "Unexpected GET_REPORT length");
    require(memcmp(e->opaque, expected, limit) == 0, "Unexpected GET_REPORT bytes");
}

int main(void)
{
    struct usb_endpoint e = {0};
    const uint8_t right[8] = {0, 0, 0x4f, 0, 0, 0, 0, 0};
    const uint8_t six[8] = {0xa5, 0, 0x1d, 0x04, 0x2a, 0x28, 0x52, 0x51};
    const uint8_t neutral[8] = {0};
    const uint8_t mouse[4] = {5, 0xfe, 3, 0xfc};
    uiapkbd_keyboard_set(0, 0x4f, 0, 0, 0, 0, 0);
    dispatch_report(&e, 0x01a1, 8, 0x00010100);
    require(e.opaque != NULL && e.max_len == 8,
            "GET_REPORT(Input) returned no keyboard state");
    require(memcmp(e.opaque, right, 8) == 0, "Held right missing in GET_REPORT");

    /* Two main-loop publications reuse both keyboard slots before EP0 IN. */
    uint8_t *snapshot = e.opaque;
    uiapkbd_keyboard_clear();
    uiapkbd_keyboard_set(0xa5, 0x1d, 0x04, 0x2a, 0x28, 0x52, 0x51);
    require(memcmp(snapshot, right, 8) == 0, "EP0 snapshot changed before control IN");
    for (int length = 0; length <= 16; ++length)
        expect_report(&e, length, 0x00010100, six, 8);
    expect_report(&e, 65535, 0x00010100, six, 8);
    uiapkbd_keyboard_clear();
    expect_report(&e, 8, 0x00010100, neutral, 8);

    uiapkbd_mouse_set(5, -2, 3, -4);
    for (int length = 0; length <= 16; ++length)
        expect_report(&e, length, 0x00000100, mouse, 4);
    snapshot = e.opaque;
    uiapkbd_mouse_set(0, 0, 0, 0);
    require(memcmp(snapshot, mouse, 4) == 0, "Mouse EP0 snapshot changed");

    const uint32_t unsupported[] = {
        0x00010101, /* report ID 1; descriptor has no IDs */
        0x00010200, /* Output */
        0x00010300, /* Feature */
        0x00020100, /* unknown interface */
        0xffff0100, /* must inspect the full interface index */
    };
    for (unsigned i = 0; i < sizeof(unsupported) / sizeof(unsupported[0]); ++i) {
        e.opaque = (uint8_t *)right; e.max_len = 8;
        dispatch_report(&e, 0x01a1, 8, unsupported[i]);
        require(e.opaque == NULL && e.max_len == 0, "Unsupported request returned stale data");
    }
    dispatch_report(&e, 0x01a1, -1, 0x00010100);
    require(e.opaque == NULL && e.max_len == 0, "Negative length accepted");
    uiapkbd_keyboard_set(0, 0x4f, 0, 0, 0, 0, 0);
    dispatch_report(&e, 0x0921, 8, 0x00010200);
    require(e.opaque == NULL && e.max_len == 0, "SET_REPORT allocated an input response");
    expect_report(&e, 8, 0x00010100, right, 8);
    require(ENDPOINTS == 3 && RV003USB_HANDLE_USER_DATA == 0,
            "Unexpected pure KBD+Mouse configuration");
#if UIAP_SNES_USB_TRACE
    struct rv003usb_internal state = {0};
    struct usb_urb request = {0x5ac1, 0x00010000, 8};
    state.eps[2].count = 0x1234;
    state.eps[2].toggle_in = 1;
    trace_in_count = 65535;
    test_keyboard_in();
    require(memcmp(sent_report, right, 8) == 0, "Trace changed interrupt input bytes");
    usb_handle_other_control_message(&e, &request, &state);
    const uint8_t trace[8] = {'T', 1, 0x4f, 1, 0, 0, 0x34, 0x12};
    require(e.max_len == 8 && memcmp(e.opaque, trace, 8) == 0,
            "Trace count, first key, toggle or ACK state incorrect");
    snapshot = e.opaque;
    uiapkbd_keyboard_clear(); test_keyboard_in();
    require(memcmp(snapshot, trace, 8) == 0, "Trace EP0 snapshot changed");
    request.wLength = 3;
    usb_handle_other_control_message(&e, &request, &state);
    require(e.max_len == 3 && e.opaque[2] == 0, "Trace release/length incorrect");
    e.opaque = NULL; e.max_len = 0;
    request.wRequestTypeLSBRequestMSB = 0x5a41; /* reject OUT */
    usb_handle_other_control_message(&e, &request, &state);
    require(e.opaque == NULL && e.max_len == 0, "Trace accepted an OUT request");
    require(RV003USB_OTHER_CONTROL == 1, "Trace control routing disabled");
    trace_control_head = 0; trace_control_count = 0;
    for (unsigned i = 0; i < 18; ++i) {
        trace_in_count = 42 + i;
        dispatch_report(&e, 0x01a1, 8, 0x00010100 | i);
    }
    request.wRequestTypeLSBRequestMSB = 0x5ac1;
    request.lValueLSBIndexMSB = 0x00010001;
    request.wLength = 136;
    usb_handle_other_control_message(&e, &request, &state);
    require(e.max_len == 136 && e.opaque[0] == 'H' && e.opaque[1] == 1 &&
            e.opaque[2] == 16, "History header/length incorrect");
    for (unsigned i = 0; i < 16; ++i) {
        const uint8_t expected[8] = {0xa1, 1, i + 2, 1, 1, 0, 44 + i, 0};
        require(memcmp(e.opaque + 8 + 8 * i, expected, 8) == 0,
                "History ring order or SETUP fields incorrect");
    }
    uint8_t history_copy[136]; memcpy(history_copy, e.opaque, 136);
    trace_note_control(0x0a21, 0x00010000);
    require(memcmp(e.opaque, history_copy, 136) == 0, "History EP0 snapshot changed");
    request.wLength = 4;
    usb_handle_other_control_message(&e, &request, &state);
    require(e.max_len == 4, "History length truncation incorrect");
    puts("PASS: bounded class SETUP history, ring wrap/order, request fields and stable EP0 copy");
    puts("PASS: trace first-slot/counter instrumentation, wrapping, ACK snapshot and read-only request");
#else
    require(RV003USB_OTHER_CONTROL == 0, "Normal build enabled trace routing");
#endif
    puts("PASS: actual GET_REPORT routing, keyboard/mouse bytes, lengths, EP0 snapshots and ignored SET_REPORT");
    return 0;
}
