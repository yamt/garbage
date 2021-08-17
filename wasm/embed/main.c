/*
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/embed_wamr.md
 */

#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "wasm_export.h"

void *
read_file(const char *path, size_t *sizep)
{
        struct stat st;
        void *p;
        size_t size;
        ssize_t ssz;
        int fd;
        int ret;

        fd = open(path, O_RDONLY);
        assert(fd >= 0);
        ret = fstat(fd, &st);
        assert(ret == 0);
        size = st.st_size;
        p = malloc(size);
        assert(p != NULL);
        ssz = read(fd, p, size);
        assert(ssz == size);
        *sizep = size;
        return p;
}

int
main(int argc, char *argv[])
{
        printf("this is a native binary\n");

        wasm_runtime_init();

        wasm_module_t module;
        char error_buf[128];
        void *p;
        size_t sz;

        p = read_file(argv[1], &sz);
        module = wasm_runtime_load(p, sz, error_buf, sizeof(error_buf));
        assert(module != NULL);

        wasm_module_inst_t module_inst;
        uint32_t stack_size = 4000;
        uint32_t heap_size = 4000;
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        assert(module_inst != NULL);

#if 1
        char *args[] = {
                "foo",
        };
        wasm_application_execute_main(module_inst, 1, args);
        /* handle exception */
#else
        wasm_function_inst_t func;
        func = wasm_runtime_lookup_function(module_inst, "main", NULL);
        assert(func != NULL);
#endif
}
