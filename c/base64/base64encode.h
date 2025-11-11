/*
 * return the size of a base64 string to encode the srclen-sized data
 */
size_t base64encode_size(size_t srclen);

/*
 * perform base64 encoding.
 *
 * dst should have enough room.
 */
void base64encode(const void *restrict src, size_t srclen, char *restrict dst);
