#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void
exported_from_main(const char *msg)
{
        printf("%s: %s\n", __func__, msg);
}

int
main(int argc, char *argv[])
{
        const char *name = argv[1];
        void *h = dlopen(name, RTLD_NOW);
        if (h == NULL) {
                fprintf(stderr, "dlopen: %s\n", dlerror());
                exit(EXIT_FAILURE);
        }
        void (*module_func)(const char *) = dlsym(h, "module_func");
        if (module_func == NULL) {
                fprintf(stderr, "dlsym: %s\n", dlerror());
                exit(EXIT_FAILURE);
        }
        module_func("hey");
}
