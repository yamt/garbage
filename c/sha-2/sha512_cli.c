#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sha512.h"

typedef uint64_t word;
#define BLOCK_SIZE SHA512_BLOCK_SIZE

void
sha512_print(const word h[8])
{
        printf("%016" PRIx64 "%016" PRIx64 "%016" PRIx64 "%016" PRIx64
               "%016" PRIx64 "%016" PRIx64 "%016" PRIx64 "%016" PRIx64 "\n",
               h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7]);
}

int
main(int argc, char **argv)
{
        uint8_t buf[BLOCK_SIZE];
        word h[8];
        uint64_t total = 0;
        unsigned int off;

        sha512_init(h);
        off = 0;
        while (1) {
                ssize_t ssz = read(STDIN_FILENO, buf + off, BLOCK_SIZE - off);
                if (ssz == -1) {
                        exit(1);
                }
                if (ssz == 0) {
                        break;
                }
                off += ssz;
                total += ssz;
                if (off >= BLOCK_SIZE) {
                        sha512_block(buf, h);
                        off = 0;
                }
        }
        sha512_tail(buf, off, total, h);
        sha512_print(h);
}
