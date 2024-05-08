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
        unsigned char *fp = __builtin_frame_address(0);
        assert(p < fp);
        unsigned char *sp = fp - margin;
        assert(p < sp);
        size_t sz = sp - p;

        memset(p, magic, sz);
}

/*
 * count the number of bytes left unmodified since fill().
 * the intention here is to count the unused stack bytes.
 *
 * XXX imprecise for windowed abis like xtensa because we have no controls
 * on register spills.
 */
__attribute__((noinline)) size_t
count(const unsigned char *p)
{
        unsigned char *fp = __builtin_frame_address(0);
        assert(p < fp);
        unsigned char *sp = fp - margin;
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

__attribute__((noinline)) unsigned char *
return_fp(void)
{
        return __builtin_frame_address(0);
}

__attribute__((noinline)) size_t
min_frame_size(void)
{
        const unsigned char *fp1 = return_fp();
        const unsigned char *fp = __builtin_frame_address(0);
        return fp - fp1;
}

char dummy[1000];

int
main(void)
{
#if defined(__APPLE__) || defined(__NuttX__)
        pthread_t self = pthread_self();
        unsigned char *addr = pthread_get_stackaddr_np(self);
        size_t sz = pthread_get_stacksize_np(self);
        unsigned char *base;
        if ((unsigned char *)&addr < (unsigned char *)addr) {
                base = addr - sz;
        } else {
                base = addr;
        }
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
        const unsigned char *fp = __builtin_frame_address(0);
        /* ensure to bind memset */
        memset(&dummy, 1, 1000);
        printf("dummy %u\n", dummy[0]);
        printf("fp %p base %p addr %p sp %p sz %zu\n", fp, base, addr, &addr,
               sz);
        printf("&addr - base = %zu\n",
               (unsigned char *)&addr - (unsigned char *)base);
        printf("margin %zu\n", margin);
        printf("min call frame size %zu\n", min_frame_size());
        printf("base sp %p\n", return_fp() - min_frame_size());
        size_t remain_base = return_fp() - min_frame_size() - base;
        printf("remaining stack before calls %zu\n", remain_base);

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
                 *
                 * for platforms like nuttx, sleep(0) might require less
                 * stack because it might not actually make a context switch.
                 */
                sleep(1);

                size_t c1 = count(base);
                printf("c0 %zu (%zu %zu) c1 %zu (%zu %zu) diff %zd\n", c0,
                       remain_base - c0, sz - c0, c1, remain_base - c1,
                       sz - c1, c0 - c1);
        }
        return 0;
}
