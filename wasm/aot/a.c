#if !defined(AOT_PTR_SIZE) && defined(__SIZEOF_POINTER__)
#define AOT_PTR_SIZE __SIZEOF_POINTER__
#endif

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char *filename;
uint64_t text_offset;
uint64_t text_size;
uint32_t module_func_count;

static bool old = false;
static bool gc = false;

/* XXX little endian is assumed */

struct hdr {
        uint32_t magic;
        uint32_t version;
};

struct shdr {
        uint32_t type;
        uint32_t size;
};

off_t
align(int fd, off_t align)
{
        off_t mask = align - 1;
        off_t off = lseek(fd, 0, SEEK_CUR);
        off_t newoff = (off + mask) & ~mask;
        lseek(fd, newoff, SEEK_SET);
        return newoff - off;
}

void
align64(int fd)
{
        align(fd, 8);
}

void
align32(int fd)
{
        align(fd, 4);
}

off_t
read64(int fd, uint64_t *p)
{
        off_t off = align(fd, 4);
        ssize_t ssz = read(fd, p, sizeof(*p));
        assert(ssz == sizeof(*p));
        return off + ssz;
}

off_t
read32(int fd, uint32_t *p)
{
        off_t off = align(fd, sizeof(*p));
        ssize_t ssz = read(fd, p, sizeof(*p));
        assert(ssz == sizeof(*p));
        return off + ssz;
}

off_t
read16(int fd, uint16_t *p)
{
        off_t off = align(fd, sizeof(*p));
        ssize_t ssz = read(fd, p, sizeof(*p));
        assert(ssz == sizeof(*p));
        return off + ssz;
}

off_t
read8(int fd, uint8_t *p)
{
        off_t off = align(fd, sizeof(*p));
        ssize_t ssz = read(fd, p, sizeof(*p));
        assert(ssz == sizeof(*p));
        return off + ssz;
}

off_t
skip(int fd, off_t sz)
{
        lseek(fd, sz, SEEK_CUR);
        return sz;
}

off_t
tell(int fd)
{
        return lseek(fd, 0, SEEK_CUR);
}

const char *
section_type(uint32_t type)
{
        switch (type) {
        case 0:
                return "target-info";
        case 1:
                return "init-data";
        case 2:
                return "text";
        case 3:
                return "function";
        case 4:
                return "export";
        case 5:
                return "relocation";
        case 6:
                return "signature";
        case 100:
                return "custom";
        }
        return "unknown";
}

off_t
readstr(int fd, char **pp)
{
        uint16_t len;
        off_t off;
        off = read16(fd, &len);
        //        printf("strlen %u\n", len);
        char *p = malloc(len + 1);
        ssize_t ssz = read(fd, p, len);
        assert(ssz == len);
        p[len] = 0;
        if (pp != NULL) {
                *pp = p;
        } else {
                printf("string: %s\n", p);
                free(p);
        }
        return off + ssz;
}

off_t
readbytes(int fd, char *buf, size_t len)
{
        ssize_t ssz = read(fd, buf, len);
        assert(ssz == len);
        return len;
}

#define DUMP_U8(name)                                                         \
        uint8_t name;                                                         \
        size -= read8(fd, &name);                                             \
        printf("%s %" PRIu8 "\n", #name, name)

#define DUMP_U16(name)                                                        \
        uint16_t name;                                                        \
        size -= read16(fd, &name);                                            \
        printf("%s %" PRIu16 "\n", #name, name)

#define DUMP_U32(name)                                                        \
        uint32_t name;                                                        \
        size -= read32(fd, &name);                                            \
        printf("%s %" PRIu32 "\n", #name, name)

#define DUMP_U64(name)                                                        \
        uint64_t name;                                                        \
        size -= read64(fd, &name);                                            \
        printf("%s %" PRIu64 "\n", #name, name)

#define DUMP_STR(name)                                                        \
        char *name;                                                           \
        size -= readstr(fd, &name);                                           \
        printf("%s %s\n", #name, name);                                       \
        free(name);

#define DUMP_INIT_EXPR() size = dump_init_expr(fd, size)

static uint32_t
dump_init_expr(int fd, uint32_t size)
{
        DUMP_U32(init_expr_type);
        switch (init_expr_type) {
        case 0: // none
        {
                break;
        }
        case 0x41: // i32
        case 0x43: // f32
        case 0xd2: // funcref
        {
                DUMP_U32(init_expr_value);
                break;
        }
        case 0x42: // i64
        case 0x44: // f64
        {
                DUMP_U64(init_expr_value);
                break;
        }
        default:
                printf("unknown init_expr_type 0x%" PRIx32 "\n",
                       init_expr_type);
                abort();
        }
        return size;
}

void
dump_target_info(int fd, size_t size)
{

        DUMP_U16(bin_type);
        DUMP_U16(abi_type);
        DUMP_U16(e_type);
        DUMP_U16(e_machine);
        DUMP_U32(e_version);
        DUMP_U32(e_flags);
        if (old) {
           DUMP_U32(reserved);
        } else {
           DUMP_U64(feature_flags);
           DUMP_U64(reserved);
        }
        char arch_str[16];
        size -= readbytes(fd, arch_str, sizeof(arch_str));
        printf("arch_str %.*s\n", (int)sizeof(arch_str), arch_str);
}

void
dump_init_data(int fd, size_t size)
{
        uint32_t i;

        DUMP_U32(import_memory_count);
        DUMP_U32(memory_count);
        for (i = 0; i < memory_count; i++) {
                DUMP_U32(memory_flags);
                DUMP_U32(num_bytes_per_page);
                DUMP_U32(mem_init_page_count);
                DUMP_U32(mem_max_page_count);
        }
        DUMP_U32(init_data_count);
        for (i = 0; i < init_data_count; i++) {
                DUMP_U32(is_passive);
                DUMP_U32(memory_index);
                if (old) {
                        DUMP_U32(init_expr_type);
                        DUMP_U64(init_expr_value);
                } else {
                        DUMP_INIT_EXPR();
                }
                DUMP_U32(byte_count);

                skip(fd, byte_count);
                size -= byte_count;
        }

        DUMP_U32(import_table_count);
        for (i = 0; i < import_table_count; i++) {
                DUMP_U32(elem_type);
                DUMP_U32(table_init_size);
                DUMP_U32(table_max_size);
                DUMP_U32(possible_grow);
        }

        DUMP_U32(table_count);
        for (i = 0; i < table_count; i++) {
                if (old) {
                        DUMP_U32(elem_type);
                        DUMP_U32(table_flags);
                        DUMP_U32(table_init_size);
                        DUMP_U32(table_max_size);
                        DUMP_U32(possible_grow);
                } else {
                        DUMP_U8(elem_type);
                        DUMP_U8(table_flags);
                        DUMP_U8(possible_grow);

                        DUMP_U32(table_init_size);
                        DUMP_U32(table_max_size);
                }
        }

        DUMP_U32(table_init_data_count);
        for (i = 0; i < table_init_data_count; i++) {
                DUMP_U32(mode);
                DUMP_U32(elem_type);
                DUMP_U32(table_index);
                DUMP_U32(init_expr_type);
                DUMP_U64(init_expr_value);

                if (old) {
                        DUMP_U32(func_index_count);
                        skip(fd, func_index_count * 4);
                        size -= func_index_count * 4;
                } else {
                        /* GC ref type info */
                        skip(fd, 8);
                        size -= 8;

                        DUMP_U32(value_count);
                        uint32_t j;
                        for (j = 0; j < value_count; j++) {
                                DUMP_INIT_EXPR();
                        }
                }
        }

        if (old) {
                DUMP_U32(func_type_count);
                for (i = 0; i < func_type_count; i++) {
                        DUMP_U32(param_count);
                        DUMP_U32(result_count);

                        uint32_t sz = param_count + result_count;
                        skip(fd, sz);
                        size -= sz;
                }
        } else {
                DUMP_U32(type_count);
                for (i = 0; i < type_count; i++) {
                        size -= align(fd, 4);
                        DUMP_U16(type_flag);
                        if (gc) {
                                DUMP_U8(is_equivalence_type);
                                if (is_equivalence_type) {
                                        DUMP_U8(pad);
                                        DUMP_U32(equiv_type_idx);
                                        continue;
                                }
                                DUMP_U8(is_sub_final);
                                DUMP_U32(parent_type_idx);
                                DUMP_U16(rec_count);
                                DUMP_U16(rec_idx);
                                switch (type_flag) {
                                case 0: // func
                                {
                                        DUMP_U16(param_count);
                                        DUMP_U16(result_count);
                                        DUMP_U16(ref_type_map_count);
                                        uint32_t sz =
                                                param_count + result_count;
                                        skip(fd, sz);
                                        size -= sz;
                                        if (ref_type_map_count) {
                                                printf("ref_type_map_count "
                                                       "not implemented\n");
                                                abort();
                                        }
                                        break;
                                }
                                default:
                                        printf("unknown type_flag 0x%" PRIx16
                                               "\n",
                                               type_flag);
                                        abort();
                                }
                        } else {
                                DUMP_U16(param_count);
                                DUMP_U16(result_count);
                                uint32_t sz = param_count + result_count;
                                skip(fd, sz);
                                size -= sz;
                        }
                }
        }

        DUMP_U32(import_global_count);
        for (i = 0; i < import_table_count; i++) {
                DUMP_U32(type);
                DUMP_U32(is_mutable);
                DUMP_STR(module_name);
                DUMP_STR(global_name);
        }

        DUMP_U32(global_count);
        for (i = 0; i < global_count; i++) {
                DUMP_U8(type);
                DUMP_U8(is_mutable);
                if (old) {
                        DUMP_U16(init_expr_type);
                        if (init_expr_type != 0xfd) {
                                DUMP_U64(init_expr);
                        } else {
                                DUMP_U64(init_expr_1);
                                DUMP_U64(init_expr_2);
                        }
                } else {
                        DUMP_INIT_EXPR();
                }
        }

        DUMP_U32(import_func_count);
        for (i = 0; i < import_func_count; i++) {
                DUMP_U16(func_type_index);
                DUMP_STR(module_name);
                DUMP_STR(func_name);
        }

        DUMP_U32(func_count);
        module_func_count = func_count;
        DUMP_U32(start_func_index);

        if (old) {
                DUMP_U32(aux_data_end_global_index);
                DUMP_U32(aux_data_end);
                DUMP_U32(aux_heap_base_global_index);
                DUMP_U32(aux_heap_base);
                DUMP_U32(aux_stack_top_global_index);
                DUMP_U32(aux_stack_bottom);
                DUMP_U32(aux_stack_size);
        } else {
                DUMP_U32(aux_data_end_global_index);
                DUMP_U64(aux_data_end);
                DUMP_U32(aux_heap_base_global_index);
                DUMP_U64(aux_heap_base);
                DUMP_U32(aux_stack_top_global_index);
                DUMP_U64(aux_stack_bottom);
                DUMP_U32(aux_stack_size);
        }

        DUMP_U32(data_section_count);
        for (i = 0; i < data_section_count; i++) {
                DUMP_STR(data_section_name);
                DUMP_U32(data_section_size);

                printf("data section [%" PRIu32 "] offset %" PRIu64
                       " size %" PRIu32 "\n",
                       i, (uint64_t)tell(fd), data_section_size);
                uint32_t sz = data_section_size;
                skip(fd, sz);
                size -= sz;
        }

        skip(fd, size);
}

void
dump_text(int fd, size_t size)
{
        uint32_t literal_size;
        size -= read32(fd, &literal_size);
        printf("literals %" PRIu32 " bytes\n", literal_size);
        skip(fd, literal_size);
        uint32_t code_size = size - literal_size;
        printf("code offset %" PRIu64 " size %" PRIu32 "\n",
               (uint64_t)tell(fd), code_size);
        printf(";; dd if=%s bs=1 iseek=%" PRIu64 " count=%" PRIu32
               " of=%s.text-section\n",
               filename, (uint64_t)tell(fd), code_size, filename);
        text_offset = (uint64_t)tell(fd);
        text_size = code_size;
        skip(fd, code_size);
}

void
dump_function(int fd, size_t size)
{
        uint32_t i;
        uint64_t prev_offset = 0;
        for (i = 0; i < module_func_count; i++) {
#if AOT_PTR_SIZE == 8
                DUMP_U64(offset);
#else
                DUMP_U32(offset);
#endif
                if (i > 0) {
                        printf(";; FUNC#%" PRIu32
                               ": dd if=%s bs=1 iseek=%" PRIu64
                               " count=%" PRIu64 " of=%s.func-%" PRIu32 "\n",
                               i - 1, filename, text_offset + prev_offset,
                               offset - prev_offset, filename, i - 1);
                }
                prev_offset = offset;
        }
        if (i > 0) {
                printf(";; FUNC#%" PRIu32 ": dd if=%s bs=1 iseek=%" PRIu64
                       " count=%" PRIu64 " of=%s.func-%" PRIu32 "\n",
                       i - 1, filename, text_offset + prev_offset,
                       text_size - prev_offset, filename, i - 1);
        }

        /*
         * ignore the rest of the section
         * as i'm not interested in it at this point
         */
        skip(fd, size);
}

void
dump_export(int fd, size_t size)
{
        uint32_t count;
        size -= read32(fd, &count);
        printf("export count %" PRIu32 "\n", count);

        while (size > 0) {
                uint32_t ind;
                uint8_t kind;
                size -= read32(fd, &ind);
                size -= read8(fd, &kind);
                printf("export index %" PRIu32 " kind %u\n", ind, kind);
                size -= readstr(fd, NULL);
        }
}

void
dump_reloc(int fd, size_t size)
{
        uint32_t count;
        uint32_t total_len;
        size -= read32(fd, &count);
        printf("reloc symbol count %" PRIu32 "\n", count);

        unsigned int i;
        for (i = 0; i < count; i++) {
                uint32_t offset;
                size -= read32(fd, &offset);
                printf("reloc symbol offset [%u] %" PRIu32 "\n", i, offset);
        }

        size -= read32(fd, &total_len);
        printf("reloc total_len %" PRIu32 "\n", total_len);
        off_t symbol_start = lseek(fd, 0, SEEK_CUR);

        for (i = 0; i < count; i++) {
                uint16_t ste;
                char *p;
                size -= readstr(fd, &p);
                printf("reloc symbol [%u] %s\n", i, p);
                free(p);
        }

        off_t symbol_end = lseek(fd, 0, SEEK_CUR);
        printf("sym start %ju sym end %ju total_len %" PRIu32 "\n",
               (uintmax_t)symbol_start, (uintmax_t)symbol_end, total_len);
        // assert(symbol_start + total_len == symbol_end);

        uint32_t group_count;
        size -= read32(fd, &group_count);
        printf("reloc group_count %" PRIu32 "\n", group_count);

        for (i = 0; i < group_count; i++) {
                uint32_t name_ind;
                size -= read32(fd, &name_ind);
                printf("reloc group [%u] name_ind %" PRIu32 "\n", i, name_ind);

                uint32_t reloc_count;
                size -= read32(fd, &reloc_count);
                printf("reloc group [%u] reloc_count %" PRIu32 "\n", i,
                       reloc_count);

                unsigned int j;
                for (j = 0; j < reloc_count; j++) {
                        uint64_t reloc_off;
                        uint64_t reloc_addend;
#if AOT_PTR_SIZE == 8
                        size -= read64(fd, &reloc_off);
                        size -= read64(fd, &reloc_addend);
#else
                        uint32_t reloc_off32;
                        uint32_t reloc_addend32;
                        size -= read32(fd, &reloc_off32);
                        size -= read32(fd, &reloc_addend32);
                        reloc_off = reloc_off32;
                        reloc_addend = reloc_addend32;
#endif
                        uint32_t reloc_type;
                        uint32_t reloc_sym_ind;
                        size -= read32(fd, &reloc_type);
                        size -= read32(fd, &reloc_sym_ind);
                        printf("reloc group [%u][%u] "
                               "offset %" PRIx64 ", "
                               "addend %" PRIx64 ", "
                               "type %" PRIu32 ", "
                               "ind %" PRIu32 "\n",
                               i, j, reloc_off, reloc_addend, reloc_type,
                               reloc_sym_ind);
                }
        }
        assert(size == 0);
}

void
dump_native(int fd, size_t *size)
{
        uint32_t count;
        *size -= read32(fd, &count);
        printf("native count %" PRIu32 "\n", count);
        unsigned int i;
        for (i = 0; i < count; i++) {
                char *p;
                *size -= readstr(fd, &p);
                printf("native symbol [%u]: %s\n", i, p);
                free(p);
        }
}

void
dump_custom(int fd, size_t size)
{
        uint32_t type;
        size -= read32(fd, &type);
        printf("custom subsec type %" PRIu32 "\n", type);
        switch (type) {
        case 1:
                dump_native(fd, &size);
                break;
        default:
                printf("custom unknown subsec type %" PRIu32 "\n", type);
                break;
        }
        if (size > 0) {
                printf("skipping %zu bytes at the end of custom section\n",
                       size);
                skip(fd, size);
        }
}

int
main(int argc, char **argv)
{
        filename = argv[1];
        int fd = open(filename, O_RDONLY);
        assert(fd != -1);
        struct hdr hdr;
        ssize_t ssz;
        ssz = read(fd, &hdr, sizeof(hdr));
        assert(ssz == sizeof(hdr));
        assert(hdr.magic == 0x746f6100);
        printf("version: %u\n", hdr.version);

        for (;;) {
                struct shdr shdr;
                align32(fd);
                ssz = read(fd, &shdr, sizeof(shdr));
                if (ssz == 0) {
                        printf("EOF\n");
                        break;
                }
                assert(ssz == sizeof(shdr));
                printf("section: %s %u %u\n", section_type(shdr.type),
                       shdr.type, shdr.size);
                switch (shdr.type) {
                case 0:
                        dump_target_info(fd, shdr.size);
                        break;
                case 1:
                        dump_init_data(fd, shdr.size);
                        break;
                case 2:
                        dump_text(fd, shdr.size);
                        break;
                case 3:
                        dump_function(fd, shdr.size);
                        break;
                case 4:
                        dump_export(fd, shdr.size);
                        break;
                case 5:
                        dump_reloc(fd, shdr.size);
                        break;
                case 100:
                        dump_custom(fd, shdr.size);
                        break;
                default:
                        skip(fd, shdr.size);
                }
        }
}
