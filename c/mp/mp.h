#if !defined(_MP_H_)
#define _MP_H_

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * internal
 */

#if !defined(BASE) && !defined(BASE_BITS)
#define BASE_BITS 32
#endif

#if BASE == 10
typedef signed char coeff_t;
#define LOG_BASE 2.30258509299404568401 /* l(BASE) */
#define COEFF_MAX 9
#define COEFF_TYPE_MAX 127
#elif BASE_BITS == 32
#include <stdint.h>

typedef uint32_t coeff_t;
#define BASE 4294967296                  /* 2^32 */
#define LOG_BASE 22.18070977791824990135 /* l(2^32) */
#define COEFF_BITS 32
#define COEFF_MAX UINT32_MAX
#define COEFF_TYPE_MAX UINT32_MAX
#elif BASE_BITS == 64
#include <stdint.h>

typedef uint64_t coeff_t;
#undef BASE /* 2^64, which is not likely representable */
#define LOG_BASE 44.36141955583649980270 /* l(2^64) */
#define COEFF_BITS 64
#define COEFF_MAX UINT64_MAX
#define COEFF_TYPE_MAX UINT64_MAX
#endif

#if 0
typedef unsigned int mp_size_t;
#define MP_SIZE_MAX UINT_MAX
#else
typedef size_t mp_size_t;
#define MP_SIZE_MAX SIZE_MAX
#endif

/*
 * ============================================================
 * the main api
 */

/*
 * a structure to represent an unsigned integer
 */
struct mpn {
        mp_size_t n;
        mp_size_t max;
        coeff_t *d;
};

#define MPN_INITIALIZER0                                                      \
        {                                                                     \
                .n = 0                                                        \
        }

#define MPN_INITIALIZER(N, ...)                                               \
        {                                                                     \
                .n = N, .d = (coeff_t[]){__VA_ARGS__},                        \
        }

/*
 * common constants
 * just for convenience
 */
extern const struct mpn g_zero;
extern const struct mpn g_one;
extern const struct mpn g_base;
extern const struct mpn g_ten;

/*
 * initialize
 */
void mpn_init(struct mpn *a);

/*
 * uninitialize
 * this frees the associated memory
 */
void mpn_clear(struct mpn *a);

/*
 * compare two values similarly to strcmp/memcmp
 */
int mpn_cmp(const struct mpn *a, const struct mpn *b);

/*
 * math
 */
int mpn_add(struct mpn *c, const struct mpn *a, const struct mpn *b);
int mpn_sub(struct mpn *c, const struct mpn *a, const struct mpn *b);
int mpn_mul(struct mpn *c, const struct mpn *a, const struct mpn *b);
int mpn_divrem(struct mpn *q, struct mpn *r, const struct mpn *a,
               const struct mpn *b);
int mpn_rootint(struct mpn *s, const struct mpn *m, unsigned int k);
int mpn_powint(struct mpn *s, const struct mpn *m, unsigned int k);

/*
 * copy
 */
int mpn_set(struct mpn *d, const struct mpn *s);

/*
 * convert from/to uintmax_t
 */
int mpn_set_uint(struct mpn *a, uintmax_t v);
int mpn_to_uint(const struct mpn *a, uintmax_t *vp);

/*
 * same as
 * "mpn_cmp(a, &g_zero) == 0" and "mpn_set(a, &g_zero)"
 */
int mpn_is_zero(const struct mpn *a);
void mpn_set_zero(struct mpn *a);

/*
 * convert to/from C string
 */
int mpn_from_str(struct mpn *a, const char *p);
int mpn_from_hex_str(struct mpn *a, const char *p);
char *mpn_to_str(const struct mpn *a);
char *mpn_to_hex_str(const struct mpn *a);
void mpn_str_free(char *p);

/*
 * internal
 */

char *mp_to_str(bool sign, const struct mpn *a);
char *mp_to_hex_str(bool sign, const struct mpn *a);

/*
 * debug
 */

bool mpn_is_normal(const struct mpn *a);
void mpn_poison(struct mpn *a);

/*
 * ============================================================
 * macros just for convenience
 * the use of these macros is completely optional
 */

#define HANDLE_ERROR(call)                                                    \
        do {                                                                  \
                ret = call;                                                   \
                if (ret != 0) {                                               \
                        goto fail;                                            \
                }                                                             \
        } while (0)

#define NO_ERROR(call)                                                        \
        do {                                                                  \
                int ret2 = call;                                              \
                assert(ret2 == 0);                                            \
        } while (0)

#define COPY_IF(cond, a, a0)                                                  \
        do {                                                                  \
                if (cond) {                                                   \
                        MPN_SET(&a0, a);                                      \
                        a = &a0;                                              \
                }                                                             \
        } while (false)

#define MPN_DEFINE(a) struct mpn a = MPN_INITIALIZER0
#define MPN_ALLOC(a, b) HANDLE_ERROR(mpn_alloc(a, b))
#define MPN_SET_UINT(a, b) HANDLE_ERROR(mpn_set_uint(a, b))
#define MPN_FROM_STR(a, b) HANDLE_ERROR(mpn_from_str(a, b))
#define MPN_TO_UINT(a, b) HANDLE_ERROR(mpn_to_uint(a, b))
#define MPN_SET(a, b) HANDLE_ERROR(mpn_set(a, b))
#define MPN_ADD(a, b, c) HANDLE_ERROR(mpn_add(a, b, c))
#define MPN_SUB(a, b, c) HANDLE_ERROR(mpn_sub(a, b, c))
#define MPN_SUB_NOFAIL(a, b, c) NO_ERROR(mpn_sub(a, b, c))
#define MPN_MUL(a, b, c) HANDLE_ERROR(mpn_mul(a, b, c))
#define MPN_DIVREM(a, b, c, d) HANDLE_ERROR(mpn_divrem(a, b, c, d))
#define MPN_ROOTINT(a, b, c) HANDLE_ERROR(mpn_rootint(a, b, c))
#define MPN_POWINT(a, b, c) HANDLE_ERROR(mpn_powint(a, b, c))

#endif /* !defined(_MP_H_) */
