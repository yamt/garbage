/*
 * a straightforward implementation of SHA-256
 *
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include <limits.h>
#include <stdint.h>

/* in SHA-256, the message size in bits must fit within a 64-bit integer */
#define SHA256_MAX_MESSAGE_BYTES (UINT64_MAX / 8)

#define SHA256_BLOCK_SIZE 64 /* block size in bytes */

/* initialize hash */
void sha256_init(uint32_t h[8]);

/* process a block (SHA256_BLOCK_SIZE byte) */
void sha256_block(const void *p, uint32_t h[8]);

/* process the last block (less than SHA256_BLOCK_SIZE byte, can be empty) */
void sha256_tail(const void *p, size_t len, uint64_t total_len, uint32_t h[8]);

/* a convenient wrapper to perform a loop using the above three functions */
void sha256(const void *vp, size_t len, uint32_t h[8]);
