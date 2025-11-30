#if !defined(_MPZ_H_)
#define _MPZ_H_

#include "mp.h"

/*
 * signed integer
 */
struct mpz {
        bool sign;
        struct mpn uint;
};

#define MPZ_INITIALIZER0                                                      \
        {                                                                     \
                .sign = 0, .uint = MPN_INITIALIZER0,                          \
        }

#define MPZ_INITIALIZER(SIGN, N, ...)                                         \
        {                                                                     \
                .sign = SIGN, .uint = MPN_INITIALIZER(N, __VA_ARGS__),        \
        }

void mpz_init(struct mpz *a);
void mpz_clear(struct mpz *a);

int mpz_cmp(const struct mpz *a, const struct mpz *b);

int mpz_add(struct mpz *c, const struct mpz *a, const struct mpz *b);
int mpz_sub(struct mpz *c, const struct mpz *a, const struct mpz *b);
int mpz_mul(struct mpz *c, const struct mpz *a, const struct mpz *b);
int mpz_divrem(struct mpz *q, struct mpz *r, const struct mpz *a,
               const struct mpz *b);

int mpz_set(struct mpz *d, const struct mpz *s);

bool mpz_is_normal(const struct mpz *a);

int mpz_from_str(struct mpz *a, const char *p);
int mpz_from_hex_str(struct mpz *a, const char *p);
char *mpz_to_str(const struct mpz *a);
char *mpz_to_hex_str(const struct mpz *a);
void mpz_str_free(char *p);

#define MPZ_DEFINE(a) struct mpz a = MPZ_INITIALIZER0
#define MPZ_ALLOC(a, b) HANDLE_ERROR(mpz_alloc(a, b))
#define MPZ_FROM_STR(a, b) HANDLE_ERROR(mpz_from_str(a, b))
#define MPZ_SET(a, b) HANDLE_ERROR(mpz_set(a, b))
#define MPZ_ADD(a, b, c) HANDLE_ERROR(mpz_add(a, b, c))
#define MPZ_SUB(a, b, c) HANDLE_ERROR(mpz_sub(a, b, c))
#define MPZ_MUL(a, b, c) HANDLE_ERROR(mpz_mul(a, b, c))
#define MPZ_DIVREM(a, b, c, d) HANDLE_ERROR(mpz_divrem(a, b, c, d))

#endif /* !defined(_MPZ_H_) */
