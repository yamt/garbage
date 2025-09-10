/* common logic between SHA-256 and SHA-512 */

static word
rotr(word v, unsigned int n)
{
        return (v >> n) | (v << (sizeof(word) * 8 - n));
}

static word
shr(word v, unsigned int n)
{
        return v >> n;
}

/* 4.1.2 */
/* 4.1.3 */
static word
ch(word x, word y, word z)
{
        return (x & y) ^ (~x & z);
}

/* 4.1.2 */
/* 4.1.3 */
static word
maj(word x, word y, word z)
{
        return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t
be32_decode(const uint8_t *p)
{
        return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
               ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint64_t
be64_decode(const uint8_t *p)
{
        return ((uint64_t)be32_decode(p) << 32) | be32_decode(p + 4);
}

static void
be32_encode(uint8_t *p, uint32_t v)
{
        p[0] = (v >> 24) & 0xff;
        p[1] = (v >> 16) & 0xff;
        p[2] = (v >> 8) & 0xff;
        p[3] = (v >> 0) & 0xff;
}

static void
be64_encode(uint8_t *p, uint64_t v)
{
        be32_encode(p, v >> 32);
        be32_encode(p + 4, v & 0xffffffff);
}
