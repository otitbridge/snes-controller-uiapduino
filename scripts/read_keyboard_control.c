// SPDX-License-Identifier: MIT
// Copyright (c) 2026 otitbridge
// Read only UIAPduino keyboard GET_REPORT(Input); no interface claim or reset.
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int property(io_service_t service, CFStringRef key) {
    int value = -1;
    CFTypeRef p = IORegistryEntryCreateCFProperty(service, key, NULL, 0);
    if (p && CFGetTypeID(p) == CFNumberGetTypeID())
        CFNumberGetValue(p, kCFNumberIntType, &value);
    if (p) CFRelease(p);
    return value;
}
int main(int argc, char **argv) {
    int history = argc == 2 && strcmp(argv[1], "--history") == 0;
    int trace = argc == 2 && strcmp(argv[1], "--trace") == 0;
    if (argc != 1 && !trace && !history) {
        fprintf(stderr, "Usage: read_keyboard_control [--trace|--history]\n");
        return 2;
    }
    io_iterator_t iterator = 0;
    IOReturn r = IOServiceGetMatchingServices(kIOMainPortDefault,
                        IOServiceMatching("IOUSBHostDevice"), &iterator);
    if (r) return 2;
    io_service_t service, target = 0;
    unsigned count = 0;
    while ((service = IOIteratorNext(iterator))) {
        if (property(service, CFSTR("idVendor")) == 0x1209 &&
            property(service, CFSTR("idProduct")) == 0xd004) {
            ++count;
            if (!target) target = service;
            else IOObjectRelease(service);
        } else IOObjectRelease(service);
    }
    IOObjectRelease(iterator);
    if (count != 1) {
        fprintf(stderr, "Expected one 1209:D004; found %u\n", count);
        if (target) IOObjectRelease(target);
        return 3;
    }
    IOCFPlugInInterface **plugin = NULL;
    SInt32 score = 0;
    r = IOCreatePlugInInterfaceForService(target, kIOUSBDeviceUserClientTypeID,
                                         kIOCFPlugInInterfaceID, &plugin, &score);
    IOObjectRelease(target);
    if (r || !plugin) { fprintf(stderr,"Plugin: 0x%08x\n",r); return 4; }
    IOUSBDeviceInterface182 **device = NULL;
    HRESULT q = (*plugin)->QueryInterface(plugin,
        CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID182), (LPVOID *)&device);
    IODestroyPlugInInterface(plugin);
    if (q || !device) { fprintf(stderr,"Query: 0x%08x\n",(unsigned)q); return 5; }
    uint8_t bytes[136] = {0};
    IOUSBDevRequestTO request = {.bmRequestType=0xa1, .bRequest=1,
        .wValue=0x0100, .wIndex=1, .wLength=8, .pData=bytes,
        .noDataTimeout=1000, .completionTimeout=1000};
    if (trace || history) {
        request.bmRequestType = 0xc1; request.bRequest = 0x5a;
        request.wValue = 0;
        if (history) { request.wValue = 1; request.wLength = sizeof(bytes); }
    }
    r = (*device)->DeviceRequestTO(device, &request);
    printf("{\"result\":\"0x%08x\",\"length\":%u,\"bytes\":[",r,(unsigned)request.wLenDone);
    const unsigned size = request.wLenDone < sizeof(bytes) ? request.wLenDone : sizeof(bytes);
    for (unsigned i=0;i<size;++i) printf("%s%u",i?",":"",bytes[i]);
    puts("]}");
    (*device)->Release(device);
    return r ? 6 : 0;
}
