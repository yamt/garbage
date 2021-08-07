#include <stdbool.h>

#define VENDOR_ID 0x0483
#define PRODUCT_ID 0x5710

void *sync_devices(void *);
void syncer_update(void);

void watch_hotplug_events(void);

void *watch_power_events(void *vp);
