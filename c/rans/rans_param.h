#if !defined(_RANS_PARAM_H_)
#define _RANS_PARAM_H_

#include <stdint.h>

typedef uint8_t sym_t;
typedef uint16_t prob_t;
typedef uint32_t I;

#define B 256       /* b in the paper */
#define L (128 * M) /* l in the paper */
#define M 65536     /* m in the paper */

/*
 * I   = {L, ..., L * B - 1}
 * I_s = {L / M * l_s, ..., B * L / M * l_s - 1}
 */

#define NSYMS 256

#endif /* !defined(_RANS_PARAM_H_) */
