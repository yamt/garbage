#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        size_t sz = 1024 * 1024;
        if (argc != 2) {
                return 1;
        }
        int n = atoi(argv[1]);
        void *p[n];
        int i;
        for (i = 0; i < n; i++) {
                p[i] = malloc(sz);
                memset(p[i], 0, sz);
        }
#if 0
        sleep(3);
        for (i = 0; i < n; i++) {
                free(p[i]);
        }
#endif
        return 0;
}
