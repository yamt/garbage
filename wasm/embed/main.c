/*
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/embed_wamr.md
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md
 */

#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wasm_export.h"

bool wasm_runtime_call_indirect(wasm_exec_env_t exec_env,
                                uint32_t element_indices, uint32_t argc,
                                uint32_t argv[]);

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
        printf("this is a native exported function \"add3\", called with env "
               "%p arsg %d\n",
               exec_env, a);
        return a + 3;
}

wasm_function_inst_t call_indirect_fn;

void *
call(wasm_exec_env_t exec_env, void *cb, void *vp)
{
        printf("this is a native exported function \"call\", called with env "
               "%p args %p "
               "%p\n",
               exec_env, cb, vp);
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
        ret = (void *)(uintptr_t)results[0].of.i32;
#else
        /*
         * XXX wasm_runtime_call_indirect is an internal function
         */
        uint32_t func = (uint32_t)(uintptr_t)cb;
        uint32_t args[1];
        args[0] = (uint32_t)(uintptr_t)vp;
        if (!wasm_runtime_call_indirect(exec_env, func, 1, args)) {
                printf("wasm_runtime_call_indirect failed\n");
                assert(false);
        }
        ret = (void *)(uintptr_t)args[0];
#endif
        printf("returning %p\n", ret);
        return ret;
}

void
prepare(wasm_exec_env_t exec_env, const char *msg, void *buf, size_t sz)
{
        snprintf(buf, sz, "<%s>", msg);
}

NativeSymbol exported_symbols[] = {
        EXPORT_WASM_API_WITH_SIG(add3, "(i)i"),
        EXPORT_WASM_API_WITH_SIG(call, "(ii)i"),
        EXPORT_WASM_API_WITH_SIG(prepare, "($*~)"),
};

void *
start_app(void *vp)
{
        wasm_module_inst_t module_inst = vp;
#if 0
        call_indirect_fn = wasm_runtime_lookup_function(module_inst,
                                                        "call_indirect", NULL);
        assert(call_indirect_fn != NULL);
#endif
#if 1
        if (!wasm_application_execute_main(module_inst, 0, NULL)) {
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
        return NULL;
}

int
main(int argc, char *argv[])
{
        printf("this is a native binary\n");

#if 1
        wasm_runtime_init();
        wasm_runtime_register_natives("env", exported_symbols, 3);
#else
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(init_args));
        init_args.native_module_name = "native_test";
        init_args.n_native_symbols = 1;
        init_args.native_symbols = exported_symbols;
        wasm_runtime_full_init(&init_args);
#endif

        char error_buf[128];
        void *p;
        size_t sz;

        char *m_argv[] = {
                "foo",
        };
        int m_argc = 1;

        p = read_file(argv[1], &sz);

        unsigned int ninst = 4;
        unsigned int i;

#if 1
        /*
         * XXX a can module be shared among instances?
         */
        wasm_module_t module;
        module = wasm_runtime_load(p, sz, error_buf, sizeof(error_buf));
        if (module == NULL) {
                printf("load: %s\n", error_buf);
                exit(1);
        }
#else
        wasm_module_t modules[ninst];
#endif

        wasm_module_inst_t module_instances[ninst];
        for (i = 0; i < ninst; i++) {
#if 0
                /*
                 * XXX a can module be shared among instances?
                 */
                wasm_module_t module;
                module =
                        wasm_runtime_load(p, sz, error_buf, sizeof(error_buf));
                if (module == NULL) {
                        printf("load [%u]: %s\n", i, error_buf);
                        exit(1);
                }
                modules[i] = module;
#endif

                const char *dirs[] = {"."};
                wasm_runtime_set_wasi_args(module, dirs, 1, NULL, 0, NULL, 0,
                                           m_argv, m_argc);

                wasm_module_inst_t module_inst;
                uint32_t stack_size = 4000;
                uint32_t heap_size = 4000;
                module_inst =
                        wasm_runtime_instantiate(module, stack_size, heap_size,
                                                 error_buf, sizeof(error_buf));
                assert(module_inst != NULL);
                module_instances[i] = module_inst;
        }

        pthread_t t[ninst];
        int ret;
        printf("embed: starting apps\n");
        for (i = 0; i < ninst; i++) {
                ret = pthread_create(&t[i], NULL, start_app,
                                     module_instances[i]);
                assert(ret == 0);
        }
        printf("embed: joining apps\n");
        for (i = 0; i < ninst; i++) {
                ret = pthread_join(t[i], NULL);
                assert(ret == 0);
        }
        printf("embed: done\n");
}
