#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
        const char *name = ".";
        if (argc > 1) {
                name = argv[1];
        }
        DIR *dir = opendir(name);
        if (dir == NULL) {
                fprintf(stderr, "opendir failed: %s: %s\n", name,
                        strerror(errno));
                exit(1);
        }
        struct dirent *d;
        while ((d = readdir(dir)) != NULL) {
                printf("%s\n", d->d_name);
        }
        closedir(dir);
}
