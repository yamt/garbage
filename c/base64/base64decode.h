/*
 * return large enough buffer size to decode a srclen-bytes base64 string.
 * it can be a few bytes larger than the decoded data.
 */
size_t base64decode_size(size_t srclen);

/*
 * return the exact size of the decoded data
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
