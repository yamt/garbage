#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

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
        void (*module_func)(const char *) = dlsym(h, "module_func");
        module_func("hey");
}
