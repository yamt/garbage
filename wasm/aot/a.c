#if !defined(AOT_PTR_SIZE) && defined(__SIZEOF_POINTER__)
#define AOT_PTR_SIZE __SIZEOF_POINTER__
#endif

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
                printf("native symbol: %s\n", p);
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
        const char *filename = argv[1];
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
