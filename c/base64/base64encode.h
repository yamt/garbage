/*
 * return the size of a base64 string to encode the srclen-sized data.
 *
 * the return value does NOT include a space for a terminating NUL.
 */
size_t base64encode_size(size_t srclen);

/*
 * perform base64 encoding.
 *
 * dst should have enough room for the encoded string, which can be
 * calculated with base64encode_size().
 *
 * this function does NOT produce a terminating NUL.
 */
void base64encode(const void *restrict src, size_t srclen, char *restrict dst);
