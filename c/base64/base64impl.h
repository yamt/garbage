#if !defined(__has_builtin)
#define __has_builtin(a) 0
#endif

#if !__has_builtin(__builtin_assume)
#define __builtin_assume(cond)
#endif

#if !defined(LITTLE_ENDIAN)
#if defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) &&                 \
                                   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define LITTLE_ENDIAN 1
#endif
#endif

#if !defined(LITTLE_ENDIAN)
#if defined(__BIG_ENDIAN__) ||                                                \
        (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define LITTLE_ENDIAN 0
#endif
#endif

#if !defined(LITTLE_ENDIAN)
#error endian is not known
#endif

#if defined(NDEBUG)
#define BASE64_ASSUME(cond) __builtin_assume(cond)
#else
#define BASE64_ASSUME(cond) assert(cond)
#endif

static uint32_t
byteswap(uint32_t x)
{
        return ((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) |
               ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff);
}
