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
        wasm_importtype_vec_delete(&imports);
}

int
compare_name_and_cstr(const wasm_name_t *name, const char *cstr)
{
        if (name->size != strlen(cstr)) {
                return 1;
        }
        return memcmp(name->data, cstr, name->size);
}

int
compare_names(const wasm_name_t *a, const wasm_name_t *b)
{
        if (a->size != b->size) {
                return 1;
        }
        return memcmp(a->data, b->data, a->size);
}

int
compare_externtype(const wasm_externtype_t *extype,
                   const wasm_externtype_t *imtype)
{
        wasm_externkind_t kind = wasm_externtype_kind(imtype);
        if (kind != wasm_externtype_kind(extype)) {
                return 1;
        }
        /* TODO: implement */
        return 0;
}

struct module {
        wasm_byte_vec_t bin;
        const char *module_name;
        wasm_module_t *module;
        wasm_instance_t *instance;
        wasm_extern_vec_t exports;
};

unsigned int
resolve_imports(wasm_importtype_vec_t *imports, unsigned int nexporters,
                const struct module *exporters, wasm_extern_vec_t *externs)
{
        unsigned int nresolved = 0;

        assert(externs->size == imports->size);
        size_t i;
        for (i = 0; i < imports->size; i++) {
                wasm_importtype_t *im = imports->data[i];
                const wasm_name_t *im_module_name = wasm_importtype_module(im);
                size_t j;
                for (j = 0; j < nexporters; j++) {
                        if (!compare_name_and_cstr(im_module_name,
                                                   exporters[j].module_name)) {
                                break;
                        }
                }
                if (j == nexporters) {
                        printf("module %.*s not found\n",
                               (int)im_module_name->size,
                               (const char *)im_module_name->data);
                        continue;
                }
                wasm_exporttype_vec_t exports;
                wasm_module_exports(exporters[j].module, &exports);
                wasm_extern_vec_t mod_externs;
                wasm_instance_exports(exporters[j].instance, &mod_externs);
                assert(exports.size == mod_externs.size);
                const wasm_name_t *im_name = wasm_importtype_name(im);
                const wasm_externtype_t *imtype = wasm_importtype_type(im);
                printf("resolving %.*s:%.*s\n", (int)im_module_name->size,
                       (const char *)im_module_name->data, (int)im_name->size,
                       (const char *)im_name->data);
                for (j = 0; j < exports.size; j++) {
                        wasm_exporttype_t *ex = exports.data[j];
                        const wasm_name_t *ex_name = wasm_exporttype_name(ex);
                        if (compare_names(im_name, ex_name)) {
                                continue;
                        }
                        const wasm_externtype_t *extype =
                                wasm_exporttype_type(ex);
                        if (compare_externtype(extype, imtype)) {
                                continue;
                        }
                        externs->data[i] =
                                wasm_extern_copy(mod_externs.data[j]);
                        nresolved++;
                        break;
                }
                wasm_exporttype_vec_delete(&exports);
                wasm_extern_vec_delete(&mod_externs);
                if (j == exports.size) {
                }
        }
        return nresolved;
}

void
load_module(wasm_store_t *store, struct module *mod, const char *fn,
            unsigned int nexporters, const struct module *exporters,
            const char *register_name)
{
        mod->module = load(fn, store, &mod->bin);
        assert(mod->module);
        wasm_importtype_vec_t imports;
        wasm_module_imports(mod->module, &imports);
        size_t size = imports.size;
        wasm_extern_vec_t externs;
        wasm_extern_vec_new_uninitialized(&externs, size);
        unsigned int nresolved =
                resolve_imports(&imports, nexporters, exporters, &externs);
        wasm_importtype_vec_delete(&imports);
        assert(nresolved == size);
        mod->instance = wasm_instance_new(store, mod->module, &externs, NULL);
        assert(mod->instance);
        wasm_extern_vec_delete(&externs);
        wasm_instance_exports(mod->instance, &mod->exports);
        mod->module_name = register_name;
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
        load_module(store, &modules[0], NAME ".0.wasm", 0, NULL, "g");
        load_module(store, &modules[1], NAME ".1.wasm", 1, modules, "sub");
        load_module(store, &modules[2], NAME ".2.wasm", 2, modules, "main");

        wasm_func_t *inc_fn = wasm_extern_as_func(modules[2].exports.data[0]);
        wasm_func_t *get_fn = wasm_extern_as_func(modules[2].exports.data[1]);
        wasm_func_t *sub_get_fn =
                wasm_extern_as_func(modules[1].exports.data[1]);

        int i;
        for (i = 0; i < nmodules; i++) {
                struct module *mod = &modules[i];
                wasm_module_delete(mod->module);
                wasm_instance_delete(mod->instance);
                wasm_byte_vec_delete(&mod->bin);
        }

        wasm_val_vec_t args;
        wasm_val_vec_new_uninitialized(&args, 0);
        wasm_val_vec_t results;
        wasm_val_vec_new_uninitialized(&results, 1);
        if (wasm_func_call(get_fn, &args, &results)) {
                assert(false);
        }
        printf("main > %u\n", (int)results.data[0].of.i32);
        if (wasm_func_call(sub_get_fn, &args, &results)) {
                assert(false);
        }
        printf("sub > %u\n", (int)results.data[0].of.i32);

        if (wasm_func_call(inc_fn, &args, &results)) {
                assert(false);
        }

        if (wasm_func_call(get_fn, &args, &results)) {
                assert(false);
        }
        printf("main > %u\n", (int)results.data[0].of.i32);
        if (wasm_func_call(sub_get_fn, &args, &results)) {
                assert(false);
        }
        printf("sub > %u\n", (int)results.data[0].of.i32);

        if (wasm_func_call(inc_fn, &args, &results)) {
                assert(false);
        }

        if (wasm_func_call(get_fn, &args, &results)) {
                assert(false);
        }
        printf("main > %u\n", (int)results.data[0].of.i32);
        if (wasm_func_call(sub_get_fn, &args, &results)) {
                assert(false);
        }
        printf("sub > %u\n", (int)results.data[0].of.i32);

        wasm_val_vec_delete(&args);
        wasm_val_vec_delete(&results);

        for (i = 0; i < nmodules; i++) {
                struct module *mod = &modules[i];
                wasm_extern_vec_delete(&mod->exports);
        }
        wasm_store_delete(store);
        wasm_engine_delete(engine);
}
