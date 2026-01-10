#include <stddef.h>
#include <stdint.h>

struct bytein {
        const uint8_t *p;
        size_t size;
};

void bytein_init(struct bytein *bi, const void *p, size_t size);
uint8_t bytein_read(struct bytein *bi);
