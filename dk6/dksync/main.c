
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dksync.h"

int
main(int argc, char *argv[])
{
        pthread_t syncer_thread;
        int ret;

        ret = pthread_create(&syncer_thread, NULL, sync_devices, NULL);
        assert(ret == 0);

        pthread_t power_watch_thread;
        ret = pthread_create(&power_watch_thread, NULL, watch_power_events,
                             NULL);
        assert(ret == 0);

        watch_hotplug_events();
}
