#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NCOND 1
#define NTH 8

struct cond {
        pthread_mutex_t *lock;
        pthread_cond_t cv;
};

__attribute__((unused)) static void *
lock_thread(void *vp)
{
        struct timespec abstime;
        struct cond *cond = vp;
        int ret;
        unsigned int delay = 1;
        unsigned int i;

        pthread_setname_np(pthread_self(), "lock", NULL);
        for (i = 0;; i++) {
                ret = clock_gettime(CLOCK_REALTIME, &abstime);
                assert(ret == 0);

                abstime.tv_nsec += delay;
                while (abstime.tv_nsec >= 1000000000) {
                        abstime.tv_sec++;
                        abstime.tv_nsec -= 1000000000;
                }
                ret = pthread_mutex_timedlock(cond->lock, &abstime);
                assert(ret == 0 || ret == ETIMEDOUT);
                if (ret == 0) {
                        ret = pthread_mutex_unlock(cond->lock);
                        assert(ret == 0);
                        delay /= 2;
                } else {
                        delay *= 2;
                }
                delay++;
                if ((i % 1000) == 0) {
                        // printf("delay=%u\n", delay);
                }
        }
        return NULL;
}

static void *
block_thread(void *vp)
{
        struct timespec abstime;
        struct cond *cond = vp;
        int ret;
        unsigned int delay = 1;
        unsigned int i;

        pthread_setname_np(pthread_self(), "block", NULL);

        ret = pthread_mutex_lock(cond->lock);
        assert(ret == 0);
        for (i = 0;; i++) {
                ret = clock_gettime(CLOCK_REALTIME, &abstime);
                assert(ret == 0);

                abstime.tv_nsec += delay;
                while (abstime.tv_nsec >= 1000000000) {
                        abstime.tv_sec++;
                        abstime.tv_nsec -= 1000000000;
                }
                ret = pthread_cond_timedwait(&cond->cv, cond->lock, &abstime);
                assert(ret == 0 || ret == ETIMEDOUT);
                if (ret == 0) {
                        delay /= 2;
                } else {
                        delay *= 2;
                }
                delay++;
                if ((i % 1000) == 0) {
                        // printf("delay=%u\n", delay);
                }
        }
        return NULL;
}

static void *
signal_thread(void *vp)
{
        struct cond *cond = vp;
        int ret;

        pthread_setname_np(pthread_self(), "signal", NULL);

        while (1) {
                ret = pthread_cond_signal(&cond->cv);
                // ret = pthread_cond_broadcast(&cond->cv);
                assert(ret == 0);
        }
        return NULL;
}

__attribute__((unused)) static void *
signal_with_lock_thread(void *vp)
{
        struct cond *cond = vp;
        int ret;

        pthread_setname_np(pthread_self(), "signal-lock", NULL);

        while (1) {
                ret = pthread_mutex_lock(cond->lock);
                assert(ret == 0);
                ret = pthread_cond_signal(&cond->cv);
                // ret = pthread_cond_broadcast(&cond->cv);
                assert(ret == 0);
                ret = pthread_mutex_unlock(cond->lock);
                assert(ret == 0);
        }
        return NULL;
}

int
main(int argc, char **argv)
{
        pthread_mutexattr_t mattr;
        pthread_mutex_t lock;
        struct cond conds[NCOND];
        unsigned int i;
        int ret;

        ret = pthread_mutexattr_init(&mattr);
        assert(ret == 0);
#if 0
		ret = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
        assert(ret == 0);
#endif
        ret = pthread_mutex_init(&lock, &mattr);
        assert(ret == 0);

        for (i = 0; i < NCOND; i++) {
                struct cond *cond = &conds[i];
                cond->lock = &lock;
                ret = pthread_cond_init(&cond->cv, NULL);
                assert(ret == 0);
        }

        pthread_t ts[4 * NTH];
        pthread_t *t = ts;
        for (i = 0; i < NTH; i++) {
                ret = pthread_create(t, NULL, block_thread, &conds[i % NCOND]);
                if (ret != 0) {
                        fprintf(stderr, "pthread_create failed witd %d\n",
                                ret);
                        exit(1);
                }
                t++;
        }

        for (i = 0; i < NTH; i++) {
                ret = pthread_create(t, NULL, lock_thread, &conds[i % NCOND]);
                if (ret != 0) {
                        fprintf(stderr, "pthread_create failed witd %d\n",
                                ret);
                        exit(1);
                }
                t++;
        }

        for (i = 0; i < NTH; i++) {
                ret = pthread_create(t, NULL, signal_with_lock_thread,
                                     &conds[i % NCOND]);
                if (ret != 0) {
                        fprintf(stderr, "pthread_create failed witd %d\n",
                                ret);
                        exit(1);
                }
                t++;
        }

        for (i = 0; i < NTH; i++) {
                ret = pthread_create(t, NULL, signal_thread,
                                     &conds[i % NCOND]);
                if (ret != 0) {
                        fprintf(stderr, "pthread_create failed witd %d\n",
                                ret);
                        exit(1);
                }
                t++;
        }

        for (i = 0; &ts[i] < t; i++) {
                void *value;
                ret = pthread_join(ts[i], &value);
                if (ret != 0) {
                        fprintf(stderr, "pthread_create failed witd %d\n",
                                ret);
                        exit(1);
                }
        }
}
