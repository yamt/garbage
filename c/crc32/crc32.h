#include <stddef.h>
#include <stdint.h>

uint32_t crc32(const void *p, size_t len);

uint32_t crc32_init(void);
uint32_t crc32_update(uint32_t crc, const void *p, size_t len);
uint32_t crc32_finalize(uint32_t crc);

uint32_t crc32_undo_update(uint32_t crc, const void *p, size_t len);
uint32_t crc32_undo_finalize(uint32_t crc);

uint32_t crc32_append(uint32_t crc1, uint32_t crc2, size_t crc2_len);

void crc32_adjust(uint32_t crc, uint32_t target_crc, uint8_t tail[4]);
