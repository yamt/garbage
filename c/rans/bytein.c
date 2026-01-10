#include <assert.h>

#include "bytein.h"

void
bytein_init(struct bytein *bi, const void *p, size_t size)
{
        bi->p = p;
        bi->size = size;
}

uint8_t
bytein_read(struct bytein *bi)
{
        assert(bi->size > 0);
        bi->size--;
        return *bi->p++;
}

