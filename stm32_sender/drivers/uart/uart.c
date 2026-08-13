#include <stdint.h>
#include "project_config.h"
#include "uart.h"
#include "rcc.h"
#include "systick.h"
#include "gpio_register.h"
#include "usart_register.h"

status_t uart_init(uint32_t baud_rate) {
    rcc_enable_usart1_clock();
    rcc_enable_gpioa_clock();

    /* Cấu hình PA9 (TX) */
    GPIOA->CRH &= ~(0xFUL << 4);
    GPIOA->CRH |= (0xBUL << 4);  

    /* Cấu hình PA10 (RX) */
    GPIOA->CRH &= ~(0xFUL << 8);
    GPIOA->CRH |= (0x4UL << 8); 

    if (baud_rate == 0) {
        return STATUS_INVALID_PARAMETER;
    }
    
    USART1->BRR = (SYSTEM_CLOCK_HZ + (baud_rate / 2)) / baud_rate;
    /* enable UART1 */
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->CR1 = (1UL << 13) | (1UL << 3) | (1UL << 2);
    
    return STATUS_OK;
}

void uart_send_byte(uint8_t byte) {
    /* Chờ cho đến khi cờ TXE (bit 7) được set lên 1 */
    while (!(USART1->SR & (1UL << 7))) {
        __asm volatile("nop");
    }
    /* Ghi byte vào thanh ghi truyền dữ liệu */
    USART1->DR = (uint32_t)byte;
}

void uart_send_buffer(const uint8_t *data, uint16_t length) {
    if (data == NULL) {
        return;
    }
    for (uint16_t i = 0; i < length; i++) {
            uart_send_byte(data[i]);
    }
}

status_t uart_receive_byte(uint8_t *byte, uint32_t timeout_ms) {
    if (byte == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    uint32_t start_tick = systick_get_tick();

    /* Chờ cờ RXNE (bit 5) được bật lên 1 */
    while (!(USART1->SR & (1UL << 5))) {
    /* Kiểm tra nếu đã quá thời gian timeout */
        if ((systick_get_tick() - start_tick) >= timeout_ms) {
            return STATUS_TIMEOUT;
        }
        __asm volatile("nop");
    }

    /* Đọc dữ liệu nhận được */
    *byte = (uint8_t)(USART1->DR & 0xFF);
    return STATUS_OK;
}