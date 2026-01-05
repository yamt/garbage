#include <assert.h>
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
        woff_t bufstart;
        woff_t curoff;     /* relative to bufstart */
        woff_t valid_size; /* relative to bufstart */
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
#if 0
        printf("lookahead_size curoff %u valid_size %u\n", s->curoff,
               s->valid_size);
#endif
        assert(s->valid_size >= s->curoff);
        woff_t sz = s->valid_size - s->curoff;
        if (sz > LOOKAHEAD_SIZE_MAX) {
                return LOOKAHEAD_SIZE_MAX;
        }
        return sz;
}

woff_t
bufidx(const struct state *s, woff_t off)
{
        return (s->bufstart + off) % BUFSIZE;
}

uint8_t
data_at(const struct state *s, woff_t off)
{
        return buf[bufidx(s, off)];
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

static bool
fill_part(struct state *s, woff_t off, woff_t len)
{
        int fd = STDIN_FILENO;
        ssize_t rsz = readall(fd, &buf[off], BUFSIZE - off);
        if (rsz == -1) {
                fprintf(stderr, "read error: %s\n", strerror(errno));
                exit(1);
        }
#if 0
        printf("filled %zu bytes (%u -> %zu) (first half)\n", rsz,
               s->valid_size, s->valid_size + rsz);
#endif
        s->valid_size += rsz;
        return rsz != 0;
}

void
fill_buffer(struct state *s)
{
        /* forget the out of window data */
        woff_t off = win_start(s);
        if (off > 0) {
                s->bufstart = bufidx(s, off);
                s->valid_size -= off;
                s->curoff -= off;
                // printf("forgot %u bytes\n", off);
        }

        assert(s->valid_size < BUFSIZE);

        /* read as much as possible */
        off = bufidx(s, s->valid_size);
        if (s->bufstart <= off) {
                if (fill_part(s, off, BUFSIZE - off)) {
                        fill_part(s, 0, s->bufstart);
                }
        } else {
                fill_part(s, off, s->bufstart - off);
        }
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
                        printf("match dist %u, len %u\n",
                               (int)s->curoff - mpos, (int)mlen);
                        s->curoff += mlen;
                }
                if (lookahead_size(s) < LOOKAHEAD_SIZE_MAX) {
                        fill_buffer(s);
                }
        }
}
