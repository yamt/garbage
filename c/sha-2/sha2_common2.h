/* common logic between SHA-256 and SHA-512 */

static void
init_w(const uint8_t *p, word w[MSG_SCHED_SIZE])
{
        /* 6.2.2 1. */
        /* 6.4.2 1. */
        unsigned int i;
        for (i = 0; i < 16; i++) {
#if WORD_SIZE == 4
                w[i] = be32_decode(&p[i * 4]);
#else
                w[i] = be64_decode(&p[i * 8]);
#endif
        }
        for (; i < MSG_SCHED_SIZE; i++) {
                w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i - 15]) + w[i - 16];
        }
}

static void
update_h(const word w[MSG_SCHED_SIZE], word h[8])
{
        word t[8]; /* a,b,c,d,e,f,g,h */
        unsigned int i;

        /* 6.2.2 2. */
        /* 6.4.2 2. */
        for (i = 0; i < 8; i++) {
                t[i] = h[i];
        }

        /* 6.2.2 3. */
        /* 6.4.2 3. */
        for (i = 0; i < MSG_SCHED_SIZE; i++) {
                word t1 = t[7] + S1(t[4]) + ch(t[4], t[5], t[6]) + K[i] + w[i];
                word t2 = S0(t[0]) + maj(t[0], t[1], t[2]);

                t[7] = t[6];
                t[6] = t[5];
                t[5] = t[4];
                t[4] = t[3] + t1;
                t[3] = t[2];
                t[2] = t[1];
                t[1] = t[0];
                t[0] = t1 + t2;
        }

        /* 6.2.2 4. */
        /* 6.4.2 4. */
        for (i = 0; i < 8; i++) {
                h[i] += t[i];
        }
}
