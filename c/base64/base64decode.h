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

#include <stddef.h>

/*
 * this library does not rely on NUL termination of 'src'.
 * 'srclen' should not include the terminating NUL.
 */

/*
 * return large enough buffer size to decode a srclen-bytes base64 string.
 * it can be a few (0-2) bytes larger than what base64decode_size_exact()
 * returns.
 *
 * this returns a large enough size even for an invalid base64 string.
 * ie. a size larger than or equal to the max possible bytes which
 * base64decode can write out to the dst buffer before it detects
 * the encoding error and returns -1.
 */
size_t base64decode_size(size_t srclen);

/*
 * return the exact size of the decoded data
 *
 * this returns a large enough size even for an invalid base64 string.
 * ie. a size larger than or equal to the max possible bytes which
 * base64decode can write out to the dst buffer before it detects
 * the encoding error and returns -1.
 */
size_t base64decode_size_exact(const void *src, size_t srclen);

/*
 * perform base64 decoding
 *
 * dst should have enough room.
 * return -1 on invalid encoding.
 */
int base64decode(const void *restrict src, size_t srclen, void *restrict dst,
                 size_t *decoded_sizep);
