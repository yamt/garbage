/*
 * https://developer.apple.com/documentation/iokit/1557114-ioregisterforsystempower
 */

#include <assert.h>
#include <stdio.h>

#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

void
cb(void *vp, io_service_t s, UInt32 msg_type, void *msg_arg)
{
	fprintf(stderr, "cb %p type %x arg %p\n", vp, (unsigned int)msg_type,
			msg_arg);

    switch (msg_type) {
    case kIOMessageSystemHasPoweredOn:
        fprintf(stderr, "kIOMessageSystemHasPoweredOn\n");
        break;
    }
}

int
main(int argc, char *argv[])
{
    io_connect_t root;
    IONotificationPortRef notification_port;
    io_object_t notifier;

    root = IORegisterForSystemPower(NULL, &notification_port, cb, &notifier);
    assert(root != MACH_PORT_NULL);

    CFRunLoopAddSource(CFRunLoopGetCurrent(),
        IONotificationPortGetRunLoopSource(notification_port),
        kCFRunLoopDefaultMode);

    CFRunLoopRun();
}
