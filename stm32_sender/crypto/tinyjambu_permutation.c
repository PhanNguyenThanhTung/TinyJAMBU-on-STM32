#include "tinyjambu_config.h"

/* Macro xử lý 32 bước NLFSR song song cho Cortex-M3 */
#define TINYJAMBU_STEPS_32(s0, s1, s2, s3, kword) do { \
    uint32_t t1 = (s1 >> 15) | (s2 << 17); \
    uint32_t t2 = (s2 >> 6)  | (s3 << 26); \
    uint32_t t3 = (s2 >> 21) | (s3 << 11); \
    uint32_t t4 = (s2 >> 27) | (s3 << 5);  \
    s0 ^= t1 ^ (~(t2 & t3)) ^ t4 ^ kword;  \
} while (0)

void tinyjambu_permutation(uint32_t state[4], const uint32_t key[4], uint32_t rounds) {
    /* Nạp trạng thái vào các thanh ghi CPU cục bộ */
    uint32_t s0 = state[0];
    uint32_t s1 = state[1];
    uint32_t s2 = state[2];
    uint32_t s3 = state[3];

    /* Mỗi lần lặp thực hiện 128 bước với 4 từ khóa key[0..3] */
    for (uint32_t i = 0; i < rounds; i += 128) {
        TINYJAMBU_STEPS_32(s0, s1, s2, s3, key[0]);
        TINYJAMBU_STEPS_32(s1, s2, s3, s0, key[1]);
        TINYJAMBU_STEPS_32(s2, s3, s0, s1, key[2]);
        TINYJAMBU_STEPS_32(s3, s0, s1, s2, key[3]);
    }

    /* Lưu ngược lại trạng thái vào bộ nhớ state */
    state[0] = s0;
    state[1] = s1;
    state[2] = s2;
    state[3] = s3;
}