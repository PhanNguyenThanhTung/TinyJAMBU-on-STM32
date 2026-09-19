#ifndef TINYJAMBU_CONFIG_H
#define TINYJAMBU_CONFIG_H

#include <stdint.h>

#define TINYJAMBU_KEY_SIZE_BYTES       16U  /* 128-bit key (4 x 32-bit words) */
#define TINYJAMBU_NONCE_SIZE_BYTES     12U  /* 96-bit nonce (3 x 32-bit words) */
#define TINYJAMBU_TAG_SIZE_BYTES       8U   /* 64-bit authentication tag (2 x 32-bit words) */
#define TINYJAMBU_STATE_WORDS          4U   /* 128-bit internal state */

/* Các chu kỳ vòng lặp hoán vị P_n */
#define TINYJAMBU_ROUNDS_384           384U
#define TINYJAMBU_ROUNDS_640           640U
#define TINYJAMBU_ROUNDS_1024          1024U

/* FrameBits */
#define TINYJAMBU_FRAME_NONCE          1U   /* 0b001 */
#define TINYJAMBU_FRAME_AD             3U   /* 0b011 */
#define TINYJAMBU_FRAME_ENCRYPT        5U   /* 0b101 */
#define TINYJAMBU_FRAME_FINAL          7U   /* 0b111 */

#endif /* TINYJAMBU_CONFIG_H */