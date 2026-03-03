#include <sys/exec_elf.h>

#include <dlfcn.h>
#include <stdio.h>

#define HANDLE_PTR(name)                                                      \
        case name:                                                            \
                printf("%16s = %p\n", #name, (const void *)a->a_v);           \
                break;

#define HANDLE_UINT(name)                                                     \
        case name:                                                            \
                printf("%16s = %ju\n", #name, (uintmax_t)a->a_v);             \
                break;

#define HANDLE_STR(name)                                                      \
        case name:                                                            \
                printf("%16s = '%s'\n", #name, (const char *)a->a_v);         \
                break;

int
main(int argc, char **argv)
{
        const AuxInfo *a = _dlauxinfo();
        for (; a->a_type != AT_NULL; a++) {
                switch (a->a_type) {
                        HANDLE_PTR(AT_BASE)
                        HANDLE_PTR(AT_ENTRY)
                        HANDLE_PTR(AT_PHDR)
                        HANDLE_PTR(AT_STACKBASE)
                        HANDLE_UINT(AT_PHENT)
                        HANDLE_UINT(AT_PHNUM)
                        HANDLE_UINT(AT_EXECFD)
                        HANDLE_UINT(AT_FLAGS)
                        HANDLE_UINT(AT_EUID)
                        HANDLE_UINT(AT_RUID)
                        HANDLE_UINT(AT_EGID)
                        HANDLE_UINT(AT_RGID)
                        HANDLE_UINT(AT_PAGESZ)
                        HANDLE_STR(AT_SUN_EXECNAME)
                default:
                        printf("unknown a_type %08x\n", a->a_type);
                        break;
                }
        }
}
