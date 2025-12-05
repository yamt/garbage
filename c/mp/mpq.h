#if !defined(_MPQ_H_)
#define _MPQ_H_

#include "mpz.h"

/*
 * rational number
 *
 * normalization:
 * - always reduced
 * - denom.sign is false
 * - 0 is 0/1
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

int mpq_set_zero(struct mpq *a);
bool mpq_is_zero(const struct mpq *a);

int mpq_reduce(struct mpq *a);

/*
 * when a = p/q,
 * mpq_sqrt(a,scale) = mpn_sqrt(p*scale^2)/mpn_sqrt(q*scale^2).
 */
int mpq_sqrt(struct mpq *s, const struct mpq *a, const struct mpn *scale);

/*
 * debug
 *
 * note: mpq_is_normal can return false positive.
 * that is, returns true while the mpq is not normal on exceptional events
 * like memory allocation failrue. it isn't a problem as far as it's used
 * only for assertions like "assert(mpq_is_normal(a))".
 */
bool mpq_is_normal(const struct mpq *a);

/*
 * note: our default string representation of mpq is fractional.
 * eg. "-987/123"
 */
int mpq_from_str(struct mpq *a, const char *p, size_t sz);
int mpq_from_strz(struct mpq *a, const char *p);

size_t mpq_estimate_str_size(const struct mpq *a);
int mpq_to_str_into_buf(char *p, size_t sz, const struct mpq *a, size_t *szp);
char *mpq_to_strz(const struct mpq *a);
void mpq_str_free(char *p);

/*
 * "decimal_fraction" functions below produce a string like "-8.02439"
 */
int mpq_estimate_decimal_fraction_str_size(const struct mpq *a,
                                           mp_size_t frac_digits);
int mpq_to_decimal_fraction_str_into_buf(char *p, size_t sz,
                                         const struct mpq *a,
                                         mp_size_t frac_digits, size_t *szp);
char *mpq_to_decimal_fraction_strz(const struct mpq *a, mp_size_t frac_digits);

#define MPQ_DEFINE(a) struct mpq a = MPQ_INITIALIZER0
#define MPQ_ALLOC(a, b) MP_HANDLE_ERROR(mpq_alloc(a, b))
#define MPQ_SET(a, b) MP_HANDLE_ERROR(mpq_set(a, b))
#define MPQ_SET_ZERO(a) MP_HANDLE_ERROR(mpq_set_zero(a))
#define MPQ_CMP(a, b, c) MP_HANDLE_ERROR(mpq_cmp(a, b, c))
#define MPQ_ADD(a, b, c) MP_HANDLE_ERROR(mpq_add(a, b, c))
#define MPQ_SUB(a, b, c) MP_HANDLE_ERROR(mpq_sub(a, b, c))
#define MPQ_MUL(a, b, c) MP_HANDLE_ERROR(mpq_mul(a, b, c))
#define MPQ_DIV(a, b, c) MP_HANDLE_ERROR(mpq_div(a, b, c))

#define MPQ_REDUCE(a) MP_HANDLE_ERROR(mpq_reduce(a))
#define MPQ_SQRT(a, b, c) MP_HANDLE_ERROR(mpq_sqrt(a, b, c))

#define MPQ_FROM_STRZ(a, b) MP_HANDLE_ERROR(mpq_from_strz(a, b))

#define MPQ_COPY_IF(cond, a, a0)                                              \
        do {                                                                  \
                if (cond) {                                                   \
                        MPQ_SET(&a0, a);                                      \
                        a = &a0;                                              \
                }                                                             \
        } while (false)

#endif /* !defined(_MPQ_H_) */
