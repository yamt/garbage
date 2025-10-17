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
int bigint_add(const struct bigint *a, const struct bigint *b,
               struct bigint *c);
int bigint_sub(const struct bigint *a, const struct bigint *b,
               struct bigint *c);
int bigint_mul(const struct bigint *a, const struct bigint *b,
               struct bigint *c);
int bigint_from_str(struct bigint *a, const char *p);
void bigint_str_free(char *p);
