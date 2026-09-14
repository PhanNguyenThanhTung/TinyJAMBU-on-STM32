#include <stdio.h>
#include <string.h>

#include "crc16.h"

static int failures = 0;

#define CHECK(condition, message)                                                \
    do {                                                                         \
        if (!(condition)) {                                                      \
            fprintf(stderr, "FAIL: %s (line %d)\n", (message), __LINE__);       \
            ++failures;                                                          \
        }                                                                        \
    } while (0)

int main(void)
{
    static const uint8_t check_value[] = "123456789";
    static const uint8_t payload[] = {0xA5, 0x00, 0x03, 0x01, 0x10, 0x20, 0x30};
    uint8_t tampered[sizeof(payload)];
    uint16_t crc;

    CHECK(crc16_calculate(check_value, 9U) == 0x29B1U,
          "CRC-16/CCITT-FALSE check value");
    CHECK(crc16_calculate(NULL, 0U) == CRC16_CCITT_INIT_VALUE,
          "empty CRC returns initial value");

    crc = crc16_calculate(payload, sizeof(payload));
    CHECK(crc16_verify(payload, sizeof(payload), crc), "valid CRC accepted");

    memcpy(tampered, payload, sizeof(payload));
    tampered[4] ^= 0x01U;
    CHECK(!crc16_verify(tampered, sizeof(tampered), crc), "tampered data rejected");

    if (failures == 0) {
        puts("CRC16 tests: PASS");
        return 0;
    }
    fprintf(stderr, "CRC16 tests: %d failure(s)\n", failures);
    return 1;
}
