typedef signed char coeff_t;
#define BASE 10

struct bigint {
        unsigned int n;
        unsigned int max;
        coeff_t *d;
};
