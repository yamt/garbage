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
