/*
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/embed_wamr.md
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md
 */

#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wasm_c_api.h"

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

wasm_trap_t *
add3(const wasm_val_t *args, wasm_val_t *results)
{
        // printf("this is a native exported function, called with %d\n", a);
        // return a + 3;
        wasm_val_copy(results, args);
        return NULL;
}

#if 0
void *
call(wasm_exec_env_t exec_env, void *cb, void *vp)
{
        printf("this is a native exported function \"call\", called with %p "
               "%p\n",
               cb, vp);
        /* XXX is there a sane way to call a wasm function via a pointer? */
        return vp;
}
#endif

int
main(int argc, char *argv[])
{
        printf("this is a native binary\n");

        wasm_engine_t *engine = wasm_engine_new();
        wasm_store_t *store = wasm_store_new(engine);

        wasm_module_t *module;
        void *p;
        size_t sz;

        p = read_file(argv[1], &sz);
        wasm_byte_vec_t bin;
        wasm_byte_vec_new_uninitialized(&bin, sz);
        memcpy(bin.data, p, sz);

        module = wasm_module_new(store, &bin);
        assert(module != NULL);

#if 0
		/* XXX */
        const char *dirs[] = {"."};
        wasm_runtime_set_wasi_args(module, dirs, 1, NULL, 0, NULL, 0, m_argv,
                                   m_argc);
#endif

        wasm_functype_t *add3_type = wasm_functype_new_1_1(
                wasm_valtype_new_i32(), wasm_valtype_new_i32());
        wasm_func_t *add3_fn = wasm_func_new(store, add3_type, add3);

        // wasm_func_t *call_fn;
        const wasm_extern_t *exported_symbols[] = {
                wasm_func_as_extern(add3_fn),
#if 0
                wasm_func_as_extern(call_fn),
#endif
                NULL,
        };

        wasm_instance_t *module_inst;
        module_inst = wasm_instance_new(store, module, exported_symbols, NULL);
        assert(module_inst != NULL);

        wasm_extern_vec_t module_exports;
        wasm_instance_exports(module_inst, &module_exports);

        wasm_func_t *main_func = wasm_extern_as_func(module_exports.data[0]);
        wasm_val_t args[] = {
                WASM_I32_VAL(0),
        };
        wasm_val_t results[] = {
                WASM_INIT_VAL,
        };

        printf("calling main\n");

        if (wasm_func_call(main_func, args, results)) {
                /* handle exception */
                printf("wasm_func_as_extern for main failed\n");
        }
}
