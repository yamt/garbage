/*
 * clang -S -O1 -emit-llvm func.c
 */

#include <stdint.h>

struct ctx {
        int64_t dummy;
        int count;
};

int
func(struct ctx *ctx, int32_t n)
{
        ctx->count++;
        if (ctx->count < n) {
                return func(ctx, n);
        }
        return ctx->count;
}
