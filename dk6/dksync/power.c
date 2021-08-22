/*
 * https://developer.apple.com/documentation/iokit/1557114-ioregisterforsystempower
 */

#include <assert.h>
#include <stdio.h>

#include <IOKit/IOMessage.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

#include "dksync.h"
#include "xlog.h"

void
cb(void *vp, io_service_t s, UInt32 msg_type, void *msg_arg)
{
        xlog_printf("cb %p type %x arg %p\n", vp, (unsigned int)msg_type,
                    msg_arg);

        switch (msg_type) {
        case kIOMessageSystemHasPoweredOn:
                xlog_printf("kIOMessageSystemHasPoweredOn\n");
                syncer_update();
                break;
        }
}

void *
watch_power_events(void *vp)
{
        io_connect_t root;
        IONotificationPortRef notification_port;
        io_object_t notifier;

        root = IORegisterForSystemPower(NULL, &notification_port, cb,
                                        &notifier);
        assert(root != MACH_PORT_NULL);

        CFRunLoopAddSource(
                CFRunLoopGetCurrent(),
                IONotificationPortGetRunLoopSource(notification_port),
                kCFRunLoopDefaultMode);

        CFRunLoopRun();
        return NULL;
}
