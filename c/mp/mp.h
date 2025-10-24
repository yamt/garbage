typedef signed char coeff_t;
#define BASE 10
#define COEFF_MAX 9
#define COEFF_TYPE_MAX 127

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
int bigint_divmod(struct bigint *q, struct bigint *r, const struct bigint *a,
                  const struct bigint *b);

int bigint_set(struct bigint *d, const struct bigint *s);
int bigint_set_uint(struct bigint *a, coeff_t v);
int bigint_mul_uint(struct bigint *d, const struct bigint *a, coeff_t b);

int bigint_from_str(struct bigint *a, const char *p);
void bigint_str_free(char *p);
