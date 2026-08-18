#ifndef TINYJAMBU_H
#define TINYJAMBU_H

#include <stdint.h>
#include "tinyjambu_config.h"
#include "project_types.h"

void tinyjambu_permutation(uint32_t state[4], const uint32_t key[4], uint32_t rounds);

/* Hàm Mã hóa và Sinh mã xác thực AEAD (Authenticated Encryption) */
status_t tinyjambu_encrypt(
    const uint8_t *key,
    const uint8_t *nonce,
    const uint8_t *associated_data,
    uint32_t ad_length,
    const uint8_t *plaintext,
    uint32_t plaintext_length,
    uint8_t *ciphertext,
    uint8_t *tag
);

/* Hàm Giải mã và Xác thực AEAD (Authenticated Decryption & Verification) */
status_t tinyjambu_decrypt_and_verify(
    const uint8_t *key,
    const uint8_t *nonce,
    const uint8_t *associated_data,
    uint32_t ad_length,
    const uint8_t *ciphertext,
    uint32_t ciphertext_length,
    const uint8_t *tag,
    uint8_t *plaintext
);

#endif /* TINYJAMBU_H */