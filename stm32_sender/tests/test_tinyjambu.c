#include <stdio.h>
#include <string.h>

#include "tinyjambu.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

static int failures = 0;

#define CHECK(condition, message)                                                \
    do {                                                                         \
        if (!(condition)) {                                                      \
            fprintf(stderr, "FAIL: %s (line %d)\n", (message), __LINE__);       \
            ++failures;                                                          \
        }                                                                        \
    } while (0)

typedef struct {
    const uint8_t *plaintext;
    uint32_t plaintext_length;
    const uint8_t *associated_data;
    uint32_t associated_data_length;
    const uint8_t *expected;
} kat_vector_t;

static const uint8_t key[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

static const uint8_t nonce[12] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B
};

static const uint8_t pt_1[] = {0x00};
static const uint8_t pt_2[] = {0x00, 0x01};
static const uint8_t pt_4[] = {0x00, 0x01, 0x02, 0x03};
static const uint8_t ad_1[] = {0x00};
static const uint8_t ad_2[] = {0x00, 0x01};
static const uint8_t ad_3[] = {0x00, 0x01, 0x02};

/* TinyJAMBU-128 official KAT values: ciphertext followed by the 8-byte tag. */
static const uint8_t kat_empty[] = {
    0x7C, 0x54, 0x56, 0xE1, 0x09, 0xB5, 0x5A, 0x3A
};
static const uint8_t kat_ad_1[] = {
    0x60, 0x7D, 0xFB, 0x91, 0xAE, 0x92, 0xD1, 0x87
};
static const uint8_t kat_ad_2[] = {
    0x9B, 0x11, 0x9B, 0x25, 0x01, 0x5A, 0x10, 0xBF
};
static const uint8_t kat_ad_3[] = {
    0xDC, 0x2F, 0xDD, 0x39, 0xA7, 0x1E, 0xEA, 0x24
};
static const uint8_t kat_pt_1[] = {
    0x02, 0xA5, 0xB1, 0x93, 0xAD, 0x57, 0x39, 0x20, 0x3E
};
static const uint8_t kat_pt_2[] = {
    0x02, 0x85, 0x6A, 0xEE, 0xA2, 0x73, 0xE4, 0x6F, 0xF9, 0x8E
};
static const uint8_t kat_pt_4[] = {
    0x02, 0x85, 0x8D, 0x83, 0x40, 0xA7, 0x4F, 0xE0, 0xAC, 0xFE, 0xFF, 0x9E
};
static const uint8_t kat_pt_4_ad_1[] = {
    0xCA, 0xE4, 0xE8, 0xF4, 0x3E, 0x58, 0xE7, 0x50, 0xB7, 0x5C, 0xC2, 0xA1
};

static void run_kat(const kat_vector_t *vector)
{
    uint8_t ciphertext[16] = {0};
    uint8_t tag[TINYJAMBU_TAG_SIZE_BYTES] = {0};
    uint8_t plaintext[16] = {0};
    uint8_t combined[24] = {0};
    status_t status;

    status = tinyjambu_encrypt(
        key,
        nonce,
        vector->associated_data,
        vector->associated_data_length,
        vector->plaintext,
        vector->plaintext_length,
        ciphertext,
        tag);
    CHECK(status == STATUS_OK, "KAT encryption status");

    memcpy(combined, ciphertext, vector->plaintext_length);
    memcpy(combined + vector->plaintext_length, tag, sizeof(tag));
    CHECK(memcmp(combined, vector->expected,
                 vector->plaintext_length + sizeof(tag)) == 0,
          "official KAT output");

    status = tinyjambu_decrypt_and_verify(
        key,
        nonce,
        vector->associated_data,
        vector->associated_data_length,
        ciphertext,
        vector->plaintext_length,
        tag,
        plaintext);
    CHECK(status == STATUS_OK, "KAT decryption status");
    if (vector->plaintext_length > 0U) {
        CHECK(memcmp(plaintext, vector->plaintext,
                     vector->plaintext_length) == 0,
              "KAT decrypted plaintext");
    }
}

static void test_authentication_failure(void)
{
    static const uint8_t message[] = {0x10, 0x20, 0x30, 0x40, 0x50};
    uint8_t ciphertext[sizeof(message)];
    uint8_t tag[TINYJAMBU_TAG_SIZE_BYTES];
    uint8_t plaintext[sizeof(message)];
    status_t status;

    status = tinyjambu_encrypt(
        key, nonce, ad_3, sizeof(ad_3), message, sizeof(message), ciphertext, tag);
    CHECK(status == STATUS_OK, "tamper fixture encryption");

    tag[0] ^= 0x01U;
    memset(plaintext, 0xA5, sizeof(plaintext));
    status = tinyjambu_decrypt_and_verify(
        key, nonce, ad_3, sizeof(ad_3), ciphertext, sizeof(ciphertext), tag, plaintext);
    CHECK(status == STATUS_AUTHENTICATION_FAILED, "tampered tag rejected");
    for (size_t index = 0U; index < sizeof(plaintext); ++index) {
        CHECK(plaintext[index] == 0U, "plaintext cleared after authentication failure");
    }
}

static void test_permutation_vector(void)
{
    uint32_t state[4] = {0x03020100U, 0x07060504U, 0x0B0A0908U, 0x0F0E0D0CU};
    static const uint32_t permutation_key[4] = {
        0x33221100U, 0x77665544U, 0xBBAA9988U, 0xFFEEDDCCU
    };
    static const uint32_t expected[4] = {
        0xD9025B75U, 0xDEA7C711U, 0xC42BFE5CU, 0x361E5016U
    };

    tinyjambu_permutation(state, permutation_key, TINYJAMBU_ROUNDS_1024);
    CHECK(memcmp(state, expected, sizeof(expected)) == 0,
          "official permutation vector");
}

int main(void)
{
    static const kat_vector_t vectors[] = {
        {NULL, 0U, NULL, 0U, kat_empty},
        {NULL, 0U, ad_1, sizeof(ad_1), kat_ad_1},
        {NULL, 0U, ad_2, sizeof(ad_2), kat_ad_2},
        {NULL, 0U, ad_3, sizeof(ad_3), kat_ad_3},
        {pt_1, sizeof(pt_1), NULL, 0U, kat_pt_1},
        {pt_2, sizeof(pt_2), NULL, 0U, kat_pt_2},
        {pt_4, sizeof(pt_4), NULL, 0U, kat_pt_4},
        {pt_4, sizeof(pt_4), ad_1, sizeof(ad_1), kat_pt_4_ad_1}
    };

    test_permutation_vector();
    for (size_t index = 0U; index < ARRAY_SIZE(vectors); ++index) {
        run_kat(&vectors[index]);
    }
    test_authentication_failure();

    if (failures == 0) {
        puts("TinyJAMBU tests: PASS");
        return 0;
    }
    fprintf(stderr, "TinyJAMBU tests: %d failure(s)\n", failures);
    return 1;
}
