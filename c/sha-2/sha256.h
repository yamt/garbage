#include <stdint.h>

void sha256_init(uint32_t h[8]);
void sha256_block(const void *p, uint32_t h[8]);
void sha256_tail(const void *p, size_t len, size_t total_len, uint32_t h[8]);

void sha256(const void *vp, size_t len, uint32_t h[8]);
