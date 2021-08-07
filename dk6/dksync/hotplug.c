
#include <assert.h>
#include <stdio.h>

#include "libusb.h"

#include "dksync.h"

int
hotplug_cb(libusb_context *ctx, libusb_device *device,
           libusb_hotplug_event event, void *user_data)
{
        struct libusb_device_descriptor desc;
        int ret;

        ret = libusb_get_device_descriptor(device, &desc);
        assert(ret == 0);
        fprintf(stderr, "hotplug: %p event %u: %04x:%04x\n", device,
                (unsigned int)event, desc.idVendor, desc.idProduct);
        if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
                syncer_update();
        }
        return 0;
}

void
watch_hotplug_events(void)
{
        libusb_context *ctx;
        int ret;

        ret = libusb_init(&ctx);
        assert(ret == 0);

        ret = libusb_hotplug_register_callback(
                ctx,
                LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                        LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                LIBUSB_HOTPLUG_ENUMERATE, VENDOR_ID, PRODUCT_ID,
                LIBUSB_HOTPLUG_MATCH_ANY, hotplug_cb, NULL, NULL);
        assert(ret == LIBUSB_SUCCESS);
        for (;;) {
                ret = libusb_handle_events_completed(ctx, NULL);
                assert(ret == 0);
        }
}
