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

struct bigint {
        size_t n;
        size_t max;
        coeff_t *d;
};

/*
 * api
 */

/* constants */
extern const struct bigint g_zero;
extern const struct bigint g_one;
extern const struct bigint g_base;
extern const struct bigint g_ten;

void bigint_init(struct bigint *a);
void bigint_clear(struct bigint *a);
int bigint_cmp(const struct bigint *a, const struct bigint *b);

/* c = a (op) b */
int bigint_add(struct bigint *c, const struct bigint *a,
               const struct bigint *b);
int bigint_sub(struct bigint *c, const struct bigint *a,
               const struct bigint *b);
int bigint_mul(struct bigint *c, const struct bigint *a,
               const struct bigint *b);
int bigint_divrem(struct bigint *q, struct bigint *r, const struct bigint *a,
                  const struct bigint *b);
int bigint_rootint(struct bigint *s, const struct bigint *m, unsigned int k);
int bigint_powint(struct bigint *s, const struct bigint *m, unsigned int k);

int bigint_set(struct bigint *d, const struct bigint *s);
int bigint_set_uint(struct bigint *a, uintmax_t v);
int bigint_to_uint(const struct bigint *a, uintmax_t *vp);

int bigint_is_zero(const struct bigint *a);
void bigint_set_zero(struct bigint *a);

int bigint_from_str(struct bigint *a, const char *p);
int bigint_from_hex_str(struct bigint *a, const char *p);
char *bigint_to_str(const struct bigint *a);
char *bigint_to_hex_str(const struct bigint *a);
void bigint_str_free(char *p);

/*
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
                        BIGINT_SET(&a0, a);                                   \
                        a = &a0;                                              \
                }                                                             \
        } while (false)
#define BIGINT_DEFINE(a) struct bigint a = BIGINT_INITIALIZER0
#define BIGINT_ALLOC(a, b) HANDLE_ERROR(bigint_alloc(a, b))
#define BIGINT_SET_UINT(a, b) HANDLE_ERROR(bigint_set_uint(a, b))
#define BIGINT_FROM_STR(a, b) HANDLE_ERROR(bigint_from_str(a, b))
#define BIGINT_TO_UINT(a, b) HANDLE_ERROR(bigint_to_uint(a, b))
#define BIGINT_SET(a, b) HANDLE_ERROR(bigint_set(a, b))
#define BIGINT_ADD(a, b, c) HANDLE_ERROR(bigint_add(a, b, c))
#define BIGINT_SUB(a, b, c) HANDLE_ERROR(bigint_sub(a, b, c))
#define BIGINT_SUB_NOFAIL(a, b, c) NO_ERROR(bigint_sub(a, b, c))
#define BIGINT_MUL(a, b, c) HANDLE_ERROR(bigint_mul(a, b, c))
#define BIGINT_DIVREM(a, b, c, d) HANDLE_ERROR(bigint_divrem(a, b, c, d))
#define BIGINT_ROOTINT(a, b, c) HANDLE_ERROR(bigint_rootint(a, b, c))
#define BIGINT_POWINT(a, b, c) HANDLE_ERROR(bigint_powint(a, b, c))
