// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
// Non-exclusive macOS input-report observation. No control requests or HID writes.
// Records OS callbacks, not a passive capture of every packet on the USB wire.
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static volatile sig_atomic_t stopping;
static unsigned long samples, invalid;
static CFMutableSetRef attached;
static double started;
static double clock_seconds(clockid_t clock) {
    struct timespec ts;
    clock_gettime(clock, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}
static void prefix(const char *event) {
    printf("{\"event\":\"%s\",\"seconds\":%.6f,\"unix_ms\":%.3f",
           event, clock_seconds(CLOCK_MONOTONIC) - started,
           clock_seconds(CLOCK_REALTIME) * 1000);
}
static void stop_reader(int number) { (void)number; stopping = 1; }
static void added(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    (void)context; (void)sender;
    if (result != kIOReturnSuccess) { ++invalid; stopping = 1; return; }
    CFSetAddValue(attached, device);
    const CFIndex count = CFSetGetCount(attached);
    prefix("attached"); printf(",\"count\":%ld}\n", (long)count);
    if (count > 1) { ++invalid; stopping = 1; }
}
static void removed(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
    (void)context; (void)result; (void)sender;
    CFSetRemoveValue(attached, device);
    prefix("removed"); printf(",\"count\":%ld}\n", (long)CFSetGetCount(attached));
}
static void input_report(void *context, IOReturn result, void *sender,
                         IOHIDReportType type, uint32_t report_id,
                         uint8_t *report, CFIndex length) {
    (void)context; (void)sender;
    if (stopping) return;
    if (result != kIOReturnSuccess || type != kIOHIDReportTypeInput ||
        report_id != 0 || length != 8) {
        ++invalid;
        prefix("invalid");
        printf(",\"result\":\"0x%08x\",\"id\":%u,\"length\":%ld}\n",
               (unsigned)result, report_id, (long)length);
        stopping = 1;
        return;
    }
    ++samples;
    prefix("sample"); printf(",\"bytes\":[");
    for (unsigned i = 0; i < 8; ++i) printf("%s%u", i ? "," : "", report[i]);
    puts("]}");
}
static void match_number(CFMutableDictionaryRef dict, CFStringRef key, int value) {
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
    CFDictionarySetValue(dict, key, number);
    CFRelease(number);
}
int main(int argc, char **argv) {
    char *end = NULL;
    long duration = argc == 2 ? strtol(argv[1], &end, 10) : 0;
    if (argc != 2 || !end || *end || duration < 1 || duration > 600) {
        fprintf(stderr, "Usage: read_keyboard_hid <seconds: 1..600>\n");
        return 2;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
    started = clock_seconds(CLOCK_MONOTONIC);
    attached = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
    IOHIDManagerRef manager = IOHIDManagerCreate(NULL, kIOHIDOptionsTypeNone);
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
        NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    match_number(dict, CFSTR(kIOHIDVendorIDKey), 0x1209);
    match_number(dict, CFSTR(kIOHIDProductIDKey), 0xD004);
    match_number(dict, CFSTR(kIOHIDPrimaryUsagePageKey), 1);
    match_number(dict, CFSTR(kIOHIDPrimaryUsageKey), 6);
    IOHIDManagerSetDeviceMatching(manager, dict);
    CFRelease(dict);
    IOHIDManagerRegisterDeviceMatchingCallback(manager, added, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(manager, removed, NULL);
    IOHIDManagerRegisterInputReportCallback(manager, input_report, NULL);
    IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOReturn result = IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
    prefix("open"); printf(",\"result\":\"0x%08x\"}\n", (unsigned)result);
    if (result == kIOReturnSuccess) {
        signal(SIGINT, stop_reader); signal(SIGTERM, stop_reader);
        prefix("ready"); puts(",\"vid\":4617,\"pid\":53252,\"usage_page\":1,\"usage\":6}");
        while (!stopping && clock_seconds(CLOCK_MONOTONIC) - started < duration)
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
    }
    IOHIDManagerUnscheduleFromRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
    CFRelease(manager); CFRelease(attached);
    prefix("finished"); printf(",\"samples\":%lu,\"invalid\":%lu}\n", samples, invalid);
    return result == kIOReturnSuccess && samples && !invalid ? 0 : 5;
}
