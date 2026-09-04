#include "crc16.h"

#include <stddef.h>

uint16_t crc16_calculate(const uint8_t *data, uint16_t length)
{
    uint16_t crc = CRC16_CCITT_INIT_VALUE;

    if ((data == NULL) && (length > 0U)) {
        return 0U;
    }

    for (uint16_t index = 0U; index < length; ++index) {
        crc ^= (uint16_t)data[index] << 8;
        for (uint8_t bit = 0U; bit < 8U; ++bit) {
            if ((crc & 0x8000U) != 0U) {
                crc = (uint16_t)((crc << 1) ^ CRC16_CCITT_POLYNOMIAL);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

bool crc16_verify(const uint8_t *data, uint16_t length, uint16_t expected_crc)
{
    if ((data == NULL) && (length > 0U)) {
        return false;
    }
    return crc16_calculate(data, length) == expected_crc;
}
