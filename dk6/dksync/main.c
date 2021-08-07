
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

        watch_hotplug_events();
}
