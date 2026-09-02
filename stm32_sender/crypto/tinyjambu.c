#include "tinyjambu.h"

#include <stddef.h>

static uint32_t load_uint32_le(const uint8_t *src)
{
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

static void store_uint32_le(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8);
    dst[2] = (uint8_t)(value >> 16);
    dst[3] = (uint8_t)(value >> 24);
}

static void secure_zero(void *data, uint32_t length)
{
    volatile uint8_t *ptr = (volatile uint8_t *)data;

    while (length-- > 0U) {
        *ptr++ = 0U;
    }
}

static void tinyjambu_setup_state(
    uint32_t state[4],
    const uint32_t key[4],
    const uint8_t *nonce,
    const uint8_t *associated_data,
    uint32_t ad_length)
{
    uint32_t offset = 0U;

    state[0] = 0U;
    state[1] = 0U;
    state[2] = 0U;
    state[3] = 0U;

    tinyjambu_permutation(state, key, TINYJAMBU_ROUNDS_1024);

    /* TinyJAMBU-128 uses P_384 for each 32-bit nonce word. */
    for (uint32_t index = 0U; index < 3U; ++index) {
        state[1] ^= (TINYJAMBU_FRAME_NONCE << 4);
        tinyjambu_permutation(state, key, TINYJAMBU_ROUNDS_384);
        state[3] ^= load_uint32_le(nonce + (index * 4U));
    }

    while ((ad_length - offset) >= 4U) {
        state[1] ^= (TINYJAMBU_FRAME_AD << 4);
        tinyjambu_permutation(state, key, TINYJAMBU_ROUNDS_384);
        state[3] ^= load_uint32_le(associated_data + offset);
        offset += 4U;
    }

    if (offset < ad_length) {
        const uint32_t partial_length = ad_length - offset;

        state[1] ^= (TINYJAMBU_FRAME_AD << 4);
        tinyjambu_permutation(state, key, TINYJAMBU_ROUNDS_384);
        for (uint32_t index = 0U; index < partial_length; ++index) {
            state[3] ^= (uint32_t)associated_data[offset + index] << (index * 8U);
        }
        state[1] ^= partial_length;
    }
}

static void tinyjambu_generate_tag(
    uint32_t state[4],
    const uint32_t key[4],
    uint8_t tag[TINYJAMBU_TAG_SIZE_BYTES])
{
    state[1] ^= (TINYJAMBU_FRAME_FINAL << 4);
    tinyjambu_permutation(state, key, TINYJAMBU_ROUNDS_1024);
    store_uint32_le(tag, state[2]);

    state[1] ^= (TINYJAMBU_FRAME_FINAL << 4);
    tinyjambu_permutation(state, key, TINYJAMBU_ROUNDS_384);
    store_uint32_le(tag + 4U, state[2]);
}

status_t tinyjambu_encrypt(
    const uint8_t *key,
    const uint8_t *nonce,
    const uint8_t *associated_data,
    uint32_t ad_length,
    const uint8_t *plaintext,
    uint32_t plaintext_length,
    uint8_t *ciphertext,
    uint8_t *tag)
{
    uint32_t key_words[4];
    uint32_t state[4];
    uint32_t offset = 0U;

    if ((key == NULL) || (nonce == NULL) || (tag == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if ((ad_length > 0U) && (associated_data == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if ((plaintext_length > 0U) && ((plaintext == NULL) || (ciphertext == NULL))) {
        return STATUS_INVALID_PARAMETER;
    }

    for (uint32_t index = 0U; index < 4U; ++index) {
        key_words[index] = load_uint32_le(key + (index * 4U));
    }
    tinyjambu_setup_state(state, key_words, nonce, associated_data, ad_length);

    while ((plaintext_length - offset) >= 4U) {
        uint32_t data;

        state[1] ^= (TINYJAMBU_FRAME_ENCRYPT << 4);
        tinyjambu_permutation(state, key_words, TINYJAMBU_ROUNDS_1024);
        data = load_uint32_le(plaintext + offset);
        state[3] ^= data;
        store_uint32_le(ciphertext + offset, state[2] ^ data);
        offset += 4U;
    }

    if (offset < plaintext_length) {
        const uint32_t partial_length = plaintext_length - offset;

        state[1] ^= (TINYJAMBU_FRAME_ENCRYPT << 4);
        tinyjambu_permutation(state, key_words, TINYJAMBU_ROUNDS_1024);
        for (uint32_t index = 0U; index < partial_length; ++index) {
            const uint8_t data = plaintext[offset + index];
            ciphertext[offset + index] =
                data ^ (uint8_t)(state[2] >> (index * 8U));
            state[3] ^= (uint32_t)data << (index * 8U);
        }
        state[1] ^= partial_length;
    }

    tinyjambu_generate_tag(state, key_words, tag);
    secure_zero(state, sizeof(state));
    secure_zero(key_words, sizeof(key_words));
    return STATUS_OK;
}

status_t tinyjambu_decrypt_and_verify(
    const uint8_t *key,
    const uint8_t *nonce,
    const uint8_t *associated_data,
    uint32_t ad_length,
    const uint8_t *ciphertext,
    uint32_t ciphertext_length,
    const uint8_t *tag,
    uint8_t *plaintext)
{
    uint32_t key_words[4];
    uint32_t state[4];
    uint8_t expected_tag[TINYJAMBU_TAG_SIZE_BYTES];
    uint32_t offset = 0U;
    uint8_t difference = 0U;

    if ((key == NULL) || (nonce == NULL) || (tag == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if ((ad_length > 0U) && (associated_data == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if ((ciphertext_length > 0U) && ((ciphertext == NULL) || (plaintext == NULL))) {
        return STATUS_INVALID_PARAMETER;
    }

    for (uint32_t index = 0U; index < 4U; ++index) {
        key_words[index] = load_uint32_le(key + (index * 4U));
    }
    tinyjambu_setup_state(state, key_words, nonce, associated_data, ad_length);

    while ((ciphertext_length - offset) >= 4U) {
        uint32_t data;

        state[1] ^= (TINYJAMBU_FRAME_ENCRYPT << 4);
        tinyjambu_permutation(state, key_words, TINYJAMBU_ROUNDS_1024);
        data = load_uint32_le(ciphertext + offset) ^ state[2];
        state[3] ^= data;
        store_uint32_le(plaintext + offset, data);
        offset += 4U;
    }

    if (offset < ciphertext_length) {
        const uint32_t partial_length = ciphertext_length - offset;

        state[1] ^= (TINYJAMBU_FRAME_ENCRYPT << 4);
        tinyjambu_permutation(state, key_words, TINYJAMBU_ROUNDS_1024);
        for (uint32_t index = 0U; index < partial_length; ++index) {
            const uint8_t data = ciphertext[offset + index] ^
                                 (uint8_t)(state[2] >> (index * 8U));
            plaintext[offset + index] = data;
            state[3] ^= (uint32_t)data << (index * 8U);
        }
        state[1] ^= partial_length;
    }

    tinyjambu_generate_tag(state, key_words, expected_tag);
    for (uint32_t index = 0U; index < TINYJAMBU_TAG_SIZE_BYTES; ++index) {
        difference |= tag[index] ^ expected_tag[index];
    }

    secure_zero(expected_tag, sizeof(expected_tag));
    secure_zero(state, sizeof(state));
    secure_zero(key_words, sizeof(key_words));

    if (difference != 0U) {
        if (plaintext != NULL) {
            secure_zero(plaintext, ciphertext_length);
        }
        return STATUS_AUTHENTICATION_FAILED;
    }

    return STATUS_OK;
}
