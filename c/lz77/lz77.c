#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MATCH_LEN_BITS 2
#define MATCH_DISTANCE_BITS 6

#define MATCH_LEN_MIN 3
#define MATCH_LEN_MAX (MATCH_LEN_MIN + (1 << MATCH_LEN_BITS) - 1)
#define MATCH_DISTANCE_MIN 1
#define MATCH_DISTANCE_MAX                                                    \
        (MATCH_DISTANCE_MIN + (1 << MATCH_DISTANCE_BITS) - 1)
#define WINDOW_SIZE_MAX MATCH_DISTANCE_MAX
#define LOOKAHEAD_SIZE_MAX MATCH_LEN_MAX
#define BUFSIZE (2 * (WINDOW_SIZE_MAX + LOOKAHEAD_SIZE_MAX))

uint8_t buf[BUFSIZE];

ssize_t
readall(int fd, void *buf, size_t sz)
{
        ssize_t rsz;
        size_t offset = 0;
        while (offset < sz) {
                rsz = read(fd, (uint8_t *)buf + offset, sz - offset);
                if (rsz == -1) {
                        if (errno == EINTR) {
                                continue;
                        }
                        break;
                }
                if (rsz == 0) {
                        break;
                }
                offset += rsz;
        }
        if (offset > 0) {
                return offset;
        }
        return rsz;
}

typedef unsigned int woff_t;

struct state {
        woff_t curoff;
        woff_t valid_size;
};

woff_t
win_start(const struct state *s)
{
        if (s->curoff > WINDOW_SIZE_MAX) {
                return s->curoff - WINDOW_SIZE_MAX;
        }
        return 0;
}

/* include the curoff */
woff_t
lookahead_size(const struct state *s)
{
        woff_t sz = s->valid_size - s->curoff;
        if (sz > LOOKAHEAD_SIZE_MAX) {
                return LOOKAHEAD_SIZE_MAX;
        }
        return sz;
}

uint8_t
data_at(struct state *s, woff_t off)
{
        return buf[off];
}

woff_t
find_match(struct state *s, woff_t *posp)
{
#if 0
        printf("find_match curoff %u win_start %u lookahead_size %u\n", s->curoff,
               win_start(s), lookahead_size(s));
#endif
        woff_t matchlen = 0;
        woff_t matchpos = 0;
        woff_t i;
        woff_t la_size = lookahead_size(s);
        for (i = win_start(s); i < s->curoff; i++) {
                woff_t mlen = 0;
                while (mlen < la_size &&
                       data_at(s, s->curoff + mlen) == data_at(s, i + mlen)) {
                        mlen++;
                }
                // printf("mlen %u pos %u\n", mlen, i);
                if (mlen >= MATCH_LEN_MIN && mlen > matchlen) {
                        matchlen = mlen;
                        matchpos = i;
                }
        }
        *posp = matchpos;
        return matchlen;
}

void
fill_buffer(struct state *s)
{
        /* forget the out of window data */
        woff_t off = win_start(s);
        if (off > 0) {
                memmove(buf, &buf[off], s->valid_size - off);
                s->valid_size -= off;
                s->curoff -= off;
        }

        int fd = STDIN_FILENO;
        ssize_t rsz =
                readall(fd, &buf[s->valid_size], BUFSIZE - s->valid_size);
        if (rsz == -1) {
                exit(1);
        }
        s->valid_size += rsz;
}

int
main(void)
{
        struct state s0;
        struct state *s = &s0;
        memset(s, 0, sizeof(*s));
        fill_buffer(s);
        while (lookahead_size(s)) {
                woff_t mpos;
                woff_t mlen = find_match(s, &mpos);
                if (mlen == 0) {
                        printf("literal \"%c\"\n",
                               (char)data_at(s, s->curoff));
                        s->curoff++;
                } else {
                        printf("match dist %u, len %u: \"%.*s\"\n",
                               (int)s->curoff - mpos, (int)mlen, (int)mlen,
                               (const char *)&buf[s->curoff]);
                        s->curoff += mlen;
                }
                if (lookahead_size(s) < LOOKAHEAD_SIZE_MAX) {
                        fill_buffer(s);
                }
        }
}
