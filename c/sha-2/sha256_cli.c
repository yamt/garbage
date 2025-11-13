/*-
 * Copyright (c)2025 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sha256.h"

typedef uint32_t word;
#define BLOCK_SIZE SHA256_BLOCK_SIZE

void
sha256_print(const word h[8])
{
        printf("%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "%08" PRIx32
               "%08" PRIx32 "%08" PRIx32 "%08" PRIx32 "\n",
               h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7]);
}

int
main(int argc, char **argv)
{
        uint8_t buf[SHA256_BLOCK_SIZE];
        word h[8];
        uint64_t total = 0;
        unsigned int off;

        sha256_init(h);
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
                        sha256_block(buf, h);
                        off = 0;
                }
        }
        sha256_tail(buf, off, total, h);
        sha256_print(h);
}
