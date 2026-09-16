#ifndef UART_H
#define UART_H

#include "project_config.h"
#include "project_types.h"
#include "stdint.h"

/* Khởi tạo USART1 với tốc độ Baudrate tùy chỉnh */
status_t uart_init(uint32_t baud_rate);

/* Gửi 1 byte dữ liệu */
void uart_send_byte(uint8_t byte);

/* Gửi một mảng data */
void uart_send_buffer(const uint8_t *data, uint16_t length);

/* Nhận 1 byte dữ liệu trong thời gian time out */
status_t uart_receive_byte(uint8_t *byte, uint32_t timeout_ms);

#endif 