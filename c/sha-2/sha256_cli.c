#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sha256.h"

void
sha256_print(const uint32_t h[8])
{
        printf("%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "%08" PRIx32
               "%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "\n",
               h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7]);
}

int
main(int argc, char **argv)
{
        uint8_t buf[64];
        uint32_t h[8];
        uint64_t total = 0;
        unsigned int off;

        sha256_init(h);
        off = 0;
        while (1) {
                ssize_t ssz = read(STDIN_FILENO, buf + off, 64 - off);
                if (ssz == -1) {
                        exit(1);
                }
                if (ssz == 0) {
                        break;
                }
                off += ssz;
                total += ssz;
                if (off >= 64) {
                        sha256_block(buf, h);
                        off = 0;
                }
        }
        sha256_tail(buf, off, total, h);
        sha256_print(h);
}
