/*
 * a straightforward implementation of SHA-512
 *
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include <stdint.h>

#define SHA512_BLOCK_SIZE 128 /* block size in bytes */

/* initialize hash */
void sha512_init(uint64_t h[8]);

/* process a block (SHA512_BLOCK_SIZE byte) */
void sha512_block(const void *p, uint64_t h[8]);

/* process the last block (less than SHA512_BLOCK_SIZE byte, can be empty) */
void sha512_tail(const void *p, size_t len, uint64_t total_len, uint64_t h[8]);

/* a convenient wrapper to perform a loop using the above three functions */
void sha512(const void *vp, size_t len, uint64_t h[8]);
