#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
        const char *filename = argv[1];
        int ret;

        ret = access(filename, F_OK);
        printf("F_OK %d\n", ret);
        ret = access(filename, R_OK);
        printf("R_OK %d\n", ret);
        ret = access(filename, X_OK);
        printf("W_OK %d\n", ret);
        ret = access(filename, W_OK);
        printf("X_OK %d\n", ret);
        ret = access(filename, R_OK|W_OK|X_OK);
        printf("R_OK|W_OK|X_OK %d\n", ret);
}
