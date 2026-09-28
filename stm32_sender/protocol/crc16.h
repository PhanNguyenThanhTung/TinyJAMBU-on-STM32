#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stdbool.h>

/* Đa thức chuẩn CRC16-CCITT */
#define CRC16_CCITT_POLYNOMIAL   0x1021U
#define CRC16_CCITT_INIT_VALUE   0xFFFFU

/* Tính toán giá trị CRC16 cho một mảng dữ liệu */
uint16_t crc16_calculate(const uint8_t *data, uint16_t length);

/* Xác thực xem mã CRC16 của gói dữ liệu có khớp không */
bool crc16_verify(const uint8_t *data, uint16_t length, uint16_t expected_crc);

#endif /* CRC16_H */
