#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const unsigned char magic = 0xaa;
const size_t margin = 32; /* for the frame of memset */

__attribute__((noinline)) void
fill(unsigned char *p)
{
        int var;
        assert(p < (unsigned char *)&var);
        unsigned char *sp = (unsigned char *)&var - margin;
        assert(p < sp);
        size_t sz = sp - p;

        memset(p, magic, sz);
}

__attribute__((noinline)) size_t
count(const unsigned char *p)
{
        int var;
        assert(p < (unsigned char *)&var);
        unsigned char *sp = (unsigned char *)&var - margin;
        assert(p < sp);
        size_t sz = sp - p;

        size_t i;
        for (i = 0; i < sz; i++) {
                if (p[i] != magic) {
                        break;
                }
        }
        return i;
}

char dummy[1000];

int
main()
{
#if defined(__APPLE__)
        pthread_t self = pthread_self();
        unsigned char *addr = pthread_get_stackaddr_np(self);
        size_t sz = pthread_get_stacksize_np(self);
        assert((unsigned char *)&addr < addr);
        unsigned char *base = addr - sz;
        assert(base < (unsigned char *)&addr);
#endif
#if defined(__linux__)
        pthread_t self = pthread_self();
        pthread_attr_t a;
        pthread_getattr_np(self, &a);
        unsigned char *addr;
        size_t sz;
        size_t gsz;
        pthread_attr_getstack(&a, (void **)&addr, &sz);
        pthread_attr_getguardsize(&a, &gsz);
        assert(addr < (unsigned char *)&addr);
        assert((unsigned char *)&addr < addr + sz);
        unsigned char *base = addr;
        assert(gsz == 0); /* XXX */
#if 0                     /* notyet */
        printf("gsz %zu\n", gsz);
        base += gsz;
        sz -= gsz;
#endif
#endif
        /* ensure to bind memset */
        memset(&dummy, 1, 1000);
        printf("dummy %u\n", dummy[0]);
        printf("base %p addr %p sp %p sz %zu\n", base, addr, &addr, sz);

        /*
         * run it three times to see the first call overhead.
         * (eg. lazy binding)
         */
        unsigned int i;
        for (i = 0; i < 3; i++) {
                fill(base);
                size_t c0 = count(base);
                assert(c0 == count(base));

                /*
                 * sleep was chosen here because it's unlikely used
                 * earlier in this program. (eg. in crt)
                 */
                sleep(0);

                size_t c1 = count(base);
                printf("c0 %zu (%zu) c1 %zu (%zu) diff %zd\n", c0, sz - c0, c1,
                       sz - c1, c0 - c1);
        }
}
