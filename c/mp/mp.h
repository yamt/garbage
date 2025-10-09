typedef int coeff_t;

struct bigint {
        unsigned int n;
        unsigned int max;
        coeff_t *d;
};

#define BASE 10
