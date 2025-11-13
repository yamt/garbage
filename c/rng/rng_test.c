/*-
 * Copyright (c)2024,2025 YAMAMOTO Takashi,
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

#undef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "rng.h"

int
main(void)
{
        struct rng rng;
        unsigned int i;
        uint32_t min;
        uint32_t max;
        uint32_t x;

        rng_init(&rng, 0);
        for (i = 0; i < 16; i++) {
                rng.state[i] = 0;
        }
        rng.state[15] = 1;
        min = UINT32_MAX;
        max = 0;
        for (i = 0; i < 9999; i++) {
                x = rng_rand_u32(&rng);
                if (min > x) {
                        min = x;
                }
                if (max < x) {
                        max = x;
                }
        }
        printf("x    %" PRIx32 "\n", x);
        printf("min  %" PRIx32 "\n", min);
        printf("max  %" PRIx32 "\n", max);
        /* https://github.com/sergiud/random/blob/cdd9562656e8e3739c7ee6a468e25c2cf60b64ee/tests/well/main.cpp#L123
         */
        assert(x == 0x4df08652);

        rng_init(&rng, 0);
        for (i = 0; i < 16; i++) {
                rng.state[i] = 1;
        }
        min = UINT32_MAX;
        max = 0;
        for (i = 0; i < 1000000000; i++) {
                x = rng_rand_u32(&rng);
                if (min > x) {
                        min = x;
                }
                if (max < x) {
                        max = x;
                }
        }
        printf("x    %" PRIx32 "\n", x);
        printf("min  %" PRIx32 "\n", min);
        printf("max  %" PRIx32 "\n", max);
        /* https://github.com/sergiud/random/blob/cdd9562656e8e3739c7ee6a468e25c2cf60b64ee/tests/well/main.cpp#L87C53-L87C63
         */
        assert(x == 0x2b3fe99e);
}
