#if !defined(_MPQ_H_)
#define _MPQ_H_

#include "mpz.h"

/*
 * rational number
 */
struct mpq {
        struct mpz numer;
        struct mpz denom;
};

#define MPQ_INITIALIZER0                                                      \
        {                                                                     \
                .numer = MPZ_INITIALIZER0, .denom = MPZ_INITIALIZER0,         \
        }

/*
 * note: mpq_init initializes to 0/0, which is not a valid number.
 * many of functions don't accept it.
 */
void mpq_init(struct mpq *a);
void mpq_clear(struct mpq *a);

/* note: mpq_cmp can fail */
int mpq_cmp(int *resultp, const struct mpq *a, const struct mpq *b);
bool mpq_eq(const struct mpq *a, const struct mpq *b);

int mpq_add(struct mpq *c, const struct mpq *a, const struct mpq *b);
int mpq_sub(struct mpq *c, const struct mpq *a, const struct mpq *b);
int mpq_mul(struct mpq *c, const struct mpq *a, const struct mpq *b);
int mpq_div(struct mpq *c, const struct mpq *a, const struct mpq *b);

int mpq_set(struct mpq *d, const struct mpq *s);

int mpq_reduce(struct mpq *a);

/*
 * note: mpq_is_normal can return false positive.
 * (returns true while the mpq is not normal.
 */
bool mpq_is_normal(const struct mpq *a);

int mpq_from_str(struct mpq *a, const char *p, size_t sz);
int mpq_from_strz(struct mpq *a, const char *p);

size_t mpq_estimate_str_size(const struct mpq *a);
int mpq_to_str_into_buf(char *p, size_t sz, const struct mpq *a, size_t *szp);
char *mpq_to_strz(const struct mpq *a);
void mpq_str_free(char *p);

#define MPQ_DEFINE(a) struct mpq a = MPQ_INITIALIZER0
#define MPQ_ALLOC(a, b) MP_HANDLE_ERROR(mpq_alloc(a, b))
#define MPQ_SET(a, b) MP_HANDLE_ERROR(mpq_set(a, b))
#define MPQ_CMP(a, b, c) MP_HANDLE_ERROR(mpq_cmp(a, b, c))
#define MPQ_ADD(a, b, c) MP_HANDLE_ERROR(mpq_add(a, b, c))
#define MPQ_SUB(a, b, c) MP_HANDLE_ERROR(mpq_sub(a, b, c))
#define MPQ_MUL(a, b, c) MP_HANDLE_ERROR(mpq_mul(a, b, c))
#define MPQ_DIV(a, b, c) MP_HANDLE_ERROR(mpq_div(a, b, c))

#define MPQ_REDUCE(a) MP_HANDLE_ERROR(mpq_reduce(a))

#define MPQ_FROM_STRZ(a, b) MP_HANDLE_ERROR(mpq_from_strz(a, b))

#define MPQ_COPY_IF(cond, a, a0)                                              \
        do {                                                                  \
                if (cond) {                                                   \
                        MPQ_SET(&a0, a);                                      \
                        a = &a0;                                              \
                }                                                             \
        } while (false)

#endif /* !defined(_MPQ_H_) */
