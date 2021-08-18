/*
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/embed_wamr.md
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md
 */

#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
add3(wasm_exec_env_t exec_env, int a)
{
        printf("this is a native exported function, called with %d\n", a);
        return a + 3;
}

wasm_function_inst_t call_indirect_fn;

void *
call(wasm_exec_env_t exec_env, void *cb, void *vp)
{
        printf("this is a native exported function \"call\", called with %p "
               "%p\n",
               cb, vp);
        void *ret;
#if 0
        /* XXX
         * is there a sane way to call a wasm function via a pointer
         * directly?
         *
         * this implementation uses a helper in the wasm code itself.
         * (call_indirect_fn)
         */
        wasm_val_t args[2];
        wasm_val_t results[1];
        args[0].kind = WASM_I32;
        args[0].of.i32 = (uint32_t)(uintptr_t)cb;
        args[1].kind = WASM_I32;
        args[1].of.i32 = (uint32_t)(uintptr_t)vp;
        if (!wasm_runtime_call_wasm_a(exec_env, call_indirect_fn, 1, results,
                                      2, args)) {
                printf("wasm_runtime_call_wasm_a failed\n");
                assert(false);
        }
        return (void *)(uintptr_t)results[0].of.i32;
#else
        wasm_module_inst_t module_inst;
        module_inst = wasm_runtime_get_module_inst(exec_env);
        wasm_function_inst_t func;
        func = wasm_runtime_lookup_function_idx(module_inst,
                                                (uint32_t)(uintptr_t)cb, NULL);
        assert(func != NULL);
        wasm_val_t args[1];
        wasm_val_t results[1];
        args[0].kind = WASM_I32;
        args[0].of.i32 = (uint32_t)(uintptr_t)vp;
        if (!wasm_runtime_call_wasm_a(exec_env, func, 1, results, 1, args)) {
                printf("wasm_runtime_call_wasm_a failed\n");
                assert(false);
        }
        ret = (void *)(uintptr_t)results[0].of.i32;
#endif
        printf("returning %p\n", ret);
        return ret;
}

NativeSymbol exported_symbols[] = {
        EXPORT_WASM_API_WITH_SIG(add3, "(i)i"),
        EXPORT_WASM_API_WITH_SIG(call, "(ii)i"),
};

int
main(int argc, char *argv[])
{
        printf("this is a native binary\n");

#if 1
        wasm_runtime_init();
        wasm_runtime_register_natives("env", exported_symbols, 2);
#else
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(init_args));
        init_args.native_module_name = "native_test";
        init_args.n_native_symbols = 1;
        init_args.native_symbols = exported_symbols;
        wasm_runtime_full_init(&init_args);
#endif

        wasm_module_t module;
        char error_buf[128];
        void *p;
        size_t sz;

        char *m_argv[] = {
                "foo",
        };
        int m_argc = 1;

        p = read_file(argv[1], &sz);
        module = wasm_runtime_load(p, sz, error_buf, sizeof(error_buf));
        assert(module != NULL);

        const char *dirs[] = {"."};
        wasm_runtime_set_wasi_args(module, dirs, 1, NULL, 0, NULL, 0, m_argv,
                                   m_argc);

        wasm_module_inst_t module_inst;
        uint32_t stack_size = 4000;
        uint32_t heap_size = 4000;
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        assert(module_inst != NULL);

        call_indirect_fn = wasm_runtime_lookup_function(module_inst,
                                                        "call_indirect", NULL);
        assert(call_indirect_fn != NULL);
#if 1
        if (!wasm_application_execute_main(module_inst, argc, argv)) {
                /* handle exception */
                printf("wasm_application_execute_main exception: %s\n",
                       wasm_runtime_get_exception(module_inst));
        }
#else
        /*
         * XXX
         * for some reasons, wasi doesn't work well in this case.
         * maybe needs to call wasi _start?
         * cf. wasm_runtime_lookup_wasi_start_function
         */
        wasm_function_inst_t func;
#if 0
        func = wasm_runtime_lookup_function(module_inst, "entry", NULL);
        assert(func != NULL);
#else
        wasm_application_execute_func(module_inst, "entry", 0, NULL);
        /* handle exception */
#endif
#endif
}
