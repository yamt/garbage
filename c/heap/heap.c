/*
 * motivation: test swap in/out
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
        if (argc != 2) {
                fprintf(stderr, "args\n");
                exit(2);
        }
        size_t sz = atol(argv[1]);
        uint8_t *p = malloc(sz);
        if (p == NULL) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
        }
        size_t i;
        printf("filling...\n");
        for (i = 0; i < sz; i += 1024) {
                *(size_t *)&p[i] = i;
        }
        printf("checking...\n");
        for (i = 0; i < sz; i += 1024) {
                size_t actual = *(size_t *)&p[i];
                size_t expected = i;

                if (i != actual) {
                        fprintf(stderr,
                                "offset %08zx, expected %08zx != actual "
                                "%08zx\n",
                                i, expected, actual);
                }
        }
}
