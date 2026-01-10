#include "byteout.h"

void
byteout_init(struct byteout *bo, void *p, size_t sz)
{
        bo->p = p;
        bo->actual = 0;
        bo->allocated = sz;
}

void
byteout_write(struct byteout *bo, uint8_t byte)
{
        if (bo->actual >= bo->allocated) {
                bo->actual++;
                return;
        }
        bo->p[bo->actual++] = byte;
}

void
rev_byteout_write(struct byteout *bo, uint8_t byte)
{
        if (bo->actual >= bo->allocated) {
                bo->actual++;
                return;
        }
        bo->p[bo->allocated - (++bo->actual)] = byte;
}

void *
rev_byteout_ptr(const struct byteout *bo)
{
        return &bo->p[bo->allocated - bo->actual];
}
