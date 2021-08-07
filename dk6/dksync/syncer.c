#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hidapi.h"

#include "dksync.h"

#define INIT_CMD 0x30
#define LAYER_PRESS_CMD 0x3e
#define LAYER_DEPRESS_CMD 0x3c
#define SYNC_CMD 0x46

#define MAX_DEVICES 2
#define MAX_PACKET_SIZE 64

struct kbd {
        pthread_t t;
        hid_device *dev;
};

struct kbd kbds[MAX_DEVICES];
unsigned int nkbds;

unsigned int syncer_gen;
unsigned int syncer_update_gen;

void
dump_packet(const unsigned char *buf, size_t len, const char *msg)
{
        unsigned int i;

        fprintf(stderr, "%s:", msg);
        for (i = 0; i < len; i++) {
                fprintf(stderr, " %02x", buf[i]);
        }
        fprintf(stderr, "\n");
}

bool
syncer_should_reboot(void)
{
        return syncer_gen != syncer_update_gen;
}

void
syncer_update(void)
{
        fprintf(stderr, "syncer_update\n");
        syncer_update_gen++;
}

void *
syncer_thread(void *vp)
{
        struct kbd *k = vp;
        unsigned char buf[MAX_PACKET_SIZE];
        unsigned int i;
        int ret;

        unsigned char init_cmd[5] = {
                INIT_CMD,
        };
        ret = hid_write(k->dev, init_cmd, sizeof(init_cmd));
        if (ret < 0) {
                return NULL;
        }
        assert(ret == sizeof(init_cmd));
        for (;;) {
                ret = hid_read_timeout(k->dev, buf, sizeof(buf), 500);
                if (ret < 0) {
                        fprintf(stderr, "hid_read_timeout failed\n");
                        break;
                }
                if (syncer_should_reboot()) {
                        break;
                }
                if (ret == 0) {
                        continue;
                }
                assert(ret == sizeof(buf));
                dump_packet(buf, ret, "read");
                unsigned char press;
                if (buf[0] == LAYER_PRESS_CMD) {
                        press = 0x03;
                } else if (buf[0] == LAYER_DEPRESS_CMD) {
                        press = 0x02;
                } else {
                        continue;
                }
                unsigned char id = buf[1];
                unsigned char flag = buf[2];
                unsigned char layer_info = buf[3];
                unsigned char sync_cmd[8] = {
                        SYNC_CMD,          0x01, press, 0x00, id, layer_info,
                        layer_info & 0x03, 0x00,
                };
                for (i = 0; i < nkbds; i++) {
                        struct kbd *k2 = &kbds[i];
                        if (k == k2) {
                                continue;
                        }
                        ret = hid_write(k2->dev, sync_cmd, sizeof(sync_cmd));
                        if (ret < 0) {
                                continue;
                        }
                        assert(ret == sizeof(sync_cmd));
                }
        }
        return NULL;
}

void *
sync_devices(void *vp)
{
        struct hid_device_info *infos;
        struct hid_device_info *info;
        unsigned int i;
        int ret;

        for (;;) {
                infos = hid_enumerate(VENDOR_ID, PRODUCT_ID);
                for (info = infos; info != NULL; info = info->next) {
                        if (info->interface_number != 1) {
                                continue; /* we don't care non controller
                                             interfaces */
                        }

                        printf("serial %ls interface_number %d path %s\n",
                               info->serial_number, info->interface_number,
                               info->path);
                        if (nkbds >= MAX_DEVICES) {
                                fprintf(stderr,
                                        "ignoring >MAX_DEVICES devices\n");
                                continue;
                        }
                        struct kbd *k = &kbds[nkbds];
                        k->dev = hid_open_path(info->path);
                        assert(k->dev != NULL);
                        nkbds++;
                }
                hid_free_enumeration(infos);
                for (i = 0; i < nkbds; i++) {
                        struct kbd *k = &kbds[i];
                        ret = pthread_create(&k->t, NULL, syncer_thread, k);
                        assert(ret == 0);
                }
                for (i = 0; i < nkbds; i++) {
                        struct kbd *k = &kbds[i];
                        void *vp;

                        ret = pthread_join(k->t, &vp);
                        assert(ret == 0);
                }
                for (i = 0; i < nkbds; i++) {
                        struct kbd *k = &kbds[i];
                        hid_close(k->dev);
                }
                nkbds = 0;
                while (!syncer_should_reboot()) {
                        sleep(1);
                }
                syncer_gen++;
        }
        return NULL;
}
