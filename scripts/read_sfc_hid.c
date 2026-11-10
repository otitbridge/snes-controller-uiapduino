// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
// macOS reader for the diagnostic vendor interface only. No HID writes.
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static volatile sig_atomic_t stopping;
static unsigned long samples, invalid;
static double started;
static const char* names[] = {"B","Y","SELECT","START","UP","DOWN",
                              "LEFT","RIGHT","A","X","L","R"};
static double seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
static void stop_reader(int number) { (void)number; stopping = 1; }

static void input_report(void* context, IOReturn result, void* sender,
                         IOHIDReportType type, uint32_t report_id,
                         uint8_t* report, CFIndex length) {
    (void)context; (void)sender;
    if (result != kIOReturnSuccess || type != kIOHIDReportTypeInput ||
        report_id != 0 || length != 8 || report[0] != 'S' ||
        report[1] != 'F' || report[2] != 1) {
        ++invalid;
        fprintf(stderr, "Invalid diagnostic report: result=0x%08x id=%u length=%ld\n",
                (unsigned)result, (unsigned)report_id, (long)length);
        stopping = 1;
        return;
    }
    const unsigned raw = report[4] | ((unsigned)report[5] << 8);
    const unsigned pressed = report[6] | ((unsigned)report[7] << 8);
    if (pressed != ((~raw) & 0x0FFF)) {
        ++invalid; stopping = 1;
        fprintf(stderr, "Raw/decoded button mismatch\n");
        return;
    }
    ++samples;
    printf("{\"event\":\"sample\",\"seconds\":%.6f,\"sequence\":%u,"
           "\"raw\":%u,\"pressed\":%u,\"buttons\":[",
           seconds() - started, (unsigned)report[3], raw, pressed);
    unsigned count = 0;
    for (unsigned bit = 0; bit < 12; ++bit)
        if (pressed & (1U << bit)) printf("%s\"%s\"", count++ ? "," : "", names[bit]);
    puts("]}");
}

static void match_number(CFMutableDictionaryRef matching, CFStringRef key, int value) {
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
    CFDictionarySetValue(matching, key, number);
    CFRelease(number);
}

int main(int argc, char** argv) {
    char* end = NULL;
    long duration = argc == 2 ? strtol(argv[1], &end, 10) : 0;
    if (argc != 2 || !end || *end || duration < 1 || duration > 600) {
        fprintf(stderr, "Usage: %s <seconds: 1..600>\n", argv[0]);
        return 2;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
    IOHIDManagerRef manager = IOHIDManagerCreate(NULL, kIOHIDOptionsTypeNone);
    CFMutableDictionaryRef matching = CFDictionaryCreateMutable(
        NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    match_number(matching, CFSTR(kIOHIDVendorIDKey), 0x1209);
    match_number(matching, CFSTR(kIOHIDProductIDKey), 0xD004);
    match_number(matching, CFSTR(kIOHIDPrimaryUsagePageKey), 0xFF00);
    match_number(matching, CFSTR(kIOHIDPrimaryUsageKey), 1);
    IOHIDManagerSetDeviceMatching(manager, matching);
    CFRelease(matching);
    CFSetRef devices = IOHIDManagerCopyDevices(manager);
    CFIndex count = devices ? CFSetGetCount(devices) : 0;
    if (devices) CFRelease(devices);
    if (count != 1) {
        fprintf(stderr, "Expected one UIAPduino vendor HID interface; found %ld\n", (long)count);
        CFRelease(manager);
        return 3;
    }
    IOReturn result = IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
    if (result != kIOReturnSuccess) {
        fprintf(stderr, "IOHIDManagerOpen failed: 0x%08x\n", (unsigned)result);
        IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
        CFRelease(manager);
        return 4;
    }
    signal(SIGINT, stop_reader); signal(SIGTERM, stop_reader);
    started = seconds();
    IOHIDManagerRegisterInputReportCallback(manager, input_report, NULL);
    IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    puts("{\"event\":\"ready\",\"vid\":4617,\"pid\":53252,\"usage_page\":65280,\"usage\":1}");
    while (!stopping && seconds() - started < (double)duration)
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
    IOHIDManagerUnscheduleFromRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
    CFRelease(manager);
    printf("{\"event\":\"finished\",\"samples\":%lu,\"invalid\":%lu}\n", samples, invalid);
    return samples && !invalid ? 0 : 5;
}
