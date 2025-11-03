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
        unsigned int n;
        unsigned int max;
        coeff_t *d;
};

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

int bigint_set(struct bigint *d, const struct bigint *s);
int bigint_set_uint(struct bigint *a, coeff_t v);
int bigint_mul_uint(struct bigint *d, const struct bigint *a, coeff_t b);

int bigint_is_zero(const struct bigint *a);
void bigint_set_zero(struct bigint *a);

int bigint_from_str(struct bigint *a, const char *p);
char *bigint_to_str(const struct bigint *a);
void bigint_str_free(char *p);
