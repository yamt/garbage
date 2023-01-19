/* assumption: WAMR_BUILD_MULTI_MODULE=0 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "fileio.h"
#if 0
#include "wasm_c_api.h"
#else
#include "wasm.h"
#endif

#define NAME "g"

wasm_module_t *
load(const char *filename, wasm_store_t *store, wasm_byte_vec_t *bin)
{
        void *p;
        size_t sz;
        int ret = map_file(filename, &p, &sz);
        assert(ret == 0);
        wasm_byte_vec_new_uninitialized(bin, sz);
        memcpy(bin->data, p, sz);
        unmap_file(p, sz);

        wasm_module_t *module = wasm_module_new(store, bin);
        return module;
}

void
print_module(wasm_module_t *mod)
{
        wasm_importtype_vec_t imports;
        wasm_module_imports(mod, &imports);
        printf("nimports %zu\n", imports.size);
        size_t i;
        for (i = 0; i < imports.size; i++) {
                wasm_importtype_t *im = imports.data[i];
                const wasm_name_t *module_name = wasm_importtype_module(im);
                const wasm_name_t *name = wasm_importtype_name(im);
                const wasm_externtype_t *type = wasm_importtype_type(im);
                wasm_externkind_t kind = wasm_externtype_kind(type);
                printf("import [%zu] %.*s:%.*s kind %u\n", i,
                       (int)module_name->size, module_name->data,
                       (int)name->size, name->data, (int)kind);
        }
}

int
compare_name_and_cstr(wasm_name_t *name, const char *cstr)
{
        if (name->size != strlen(cstr)) {
                return 1;
        }
        return memcmp(name->data, cstr, name->size);
}

int
compare_names(wasm_name_t *a, wasm_name_t *b)
{
        if (a->size != b->size) {
                return 1;
        }
        return memcmp(a->data, b->data, a->size);
}

struct module {
        wasm_byte_vec_t bins;
        const char *module_name;
        wasm_module_t *module;
        wasm_instance_t *instance;
        wasm_extern_vec_t exports;
};

unsigned int
resolve_imports(wasm_module_t *importer, unsigned int nexporters,
                const struct module *exporters, wasm_extern_vec_t *extern)
{
        unsigned int nresolved = 0;

        wasm_importtype_vec_t imports;
        wasm_module_imports(importer, &imports);

        wasm_exporttype_vec_t exports;
        wasm_module_exports(exporter, &exports);

        assert(extern->size == imports.size);
        size_t i;
        for (i = 0; i < imports.size; i++) {
                wasm_importtype_t *im = imports.data[i];
                const wasm_name_t *im_module_name = wasm_importtype_module(im);
                if (compare_name_and_cstr(im_module_name,
                                          exporter_module_name)) {
                        continue;
                }
                const wasm_name_t *im_name = wasm_importtype_name(im);
                const wasm_externtype_t *type = wasm_importtype_type(im);
                wasm_externkind_t kind = wasm_externtype_kind(type);
                size_t j;
                for (j = 0; j < exports.size; j++) {
                        wasm_exporttype_t *ex = exports.data[i];
                        const wasm_name_t *ex_name = wasm_exporttype_name(ex);
                        if (compare_names(im_name, ex_name)) {
                                continue;
                        }
                        extern[i] = wasm_extern_copy(exports->data[j]);
                        nresolved++;
                }
        }
        return nresolved;
}

void
load_module(wasm_store_t *store, struct module *mod, const char *fn,
            wasm_extern_vec_t *externs, const char *register_name)
{
        mod->module = load(fn, store, &mod->bins);
        assert(mod->module);
        mod->instance = wasm_instance_new(store, mod->modules, externs, NULL);
        assert(mod->instance);
        wasm_instance_exports(mod->instance, &mod->exports);
}

int
main(int argc, char **argv)
{
        wasm_engine_t *engine = wasm_engine_new();
        assert(engine);
        wasm_store_t *store = wasm_store_new(engine);
        assert(store);

        const int nmodules = 3;
        struct module modules[nmodules];
        wasm_extern_vec_t empty = WASM_EMPTY_VEC;
        load_module(store, &modules[0], NAME ".0.wasm", &empty, "g");
        load_module(store, &modules[1], NAME ".1.wasm", , "sub");
        load_module(store, &modules[1], NAME ".2.wasm", , "main");

        wasm_func_t *get_fn = wasm_extern_as_func(exports4.data[1]);
#if 0
    wasm_func_t *inc_fn = wasm_extern_as_func(exports4.data[0]);
#else
        wasm_func_t *inc_fn = sub_inc_fn;
#endif

        if (wasm_func_call(get_fn, &args, &results)) {
                assert(false);
        }
        printf("main > %u\n", (int)rs[0].of.i32);
        if (wasm_func_call(sub_get_fn, &args, &results)) {
                assert(false);
        }
        printf("sub > %u\n", (int)rs[0].of.i32);

        if (wasm_func_call(inc_fn, &args, &results)) {
                assert(false);
        }

        if (wasm_func_call(get_fn, &args, &results)) {
                assert(false);
        }
        printf("main > %u\n", (int)rs[0].of.i32);
        if (wasm_func_call(sub_get_fn, &args, &results)) {
                assert(false);
        }
        printf("sub > %u\n", (int)rs[0].of.i32);

        if (wasm_func_call(inc_fn, &args, &results)) {
                assert(false);
        }

        if (wasm_func_call(get_fn, &args, &results)) {
                assert(false);
        }
        printf("main > %u\n", (int)rs[0].of.i32);
        if (wasm_func_call(sub_get_fn, &args, &results)) {
                assert(false);
        }
        printf("sub > %u\n", (int)rs[0].of.i32);
}
