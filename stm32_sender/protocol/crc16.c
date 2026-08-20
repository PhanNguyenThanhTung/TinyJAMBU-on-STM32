#include "crc16.h"
#include <stddef.h>

uint16_t crc16_calculate(const uint8_t *data, uint16_t length) {
    if (data == NULL || length == 0) {
        return 0;
    }

    uint16_t crc = CRC16_CCITT_INIT_VALUE;

    for (uint16_t i = 0; i < length; i++) {
        /* Đưa byte dữ liệu vào 8-bit cao của thanh ghi CRC 16-bit */
        crc ^= ((uint16_t)data[i] << 8);

        /* Xử lý tuần tự từng bit trong 8 bit */
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000U) {
                /* Nếu bit 15 là 1: dịch trái và XOR với đa thức 0x1021 */
                crc = (crc << 1) ^ CRC16_CCITT_POLYNOMIAL;
            } else {
                /* Nếu bit 15 là 0: chỉ dịch trái */
                crc = (crc << 1);
            }
        }
    }

    return crc;
}

bool crc16_verify(const uint8_t *data, uint16_t length, uint16_t expected_crc) {
    uint16_t calculated_crc = crc16_calculate(data, length);
    return (calculated_crc == expected_crc);
}
