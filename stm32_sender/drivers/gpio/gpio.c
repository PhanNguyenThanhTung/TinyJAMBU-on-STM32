#include "gpio.h"
#include <stddef.h>

status_t gpio_init_pin(gpio_register_t *port, const gpio_config_t *config) {
    if (port == NULL || config == NULL || config->pin > 15) {
        return STATUS_INVALID_PARAMETER;
    }

    uint8_t pin = config->pin;
    uint32_t pin_val = 0;

    /* Xác định 4-bit cấu hình [CNF : MODE] */
    if (config->speed == GPIO_SPEED_INPUT) {
        if (config->mode == GPIO_MODE_INPUT_PULLUP) {
            pin_val = 0x08;
            port->BSRR = (1UL << pin); /* Kéo điện trở Pull-up */
        } else if (config->mode == GPIO_MODE_INPUT_PULLDOWN) {
            pin_val = 0x08;
            port->BRR = (1UL << pin);  /* Kéo điện trở Pull-down */
        } else {
            pin_val = (uint32_t)config->mode;
        }
    } else {
        /* Output: Gộp 2-bit CNF và 2-bit MODE */
        uint32_t mode_bits = (uint32_t)config->speed & 0x03; /* 03 = 0000 0011 */
        uint32_t cnf_bits  = (uint32_t)config->mode & 0x0C; /* 0C = 0000 1100 */
        pin_val = (cnf_bits | mode_bits);
    }

    /* Ghi vào CRL (cho Pin 0-7) hoặc CRH (cho Pin 8-15) */
    if (pin < 8) {
        uint8_t shift = pin * 4;
        port->CRL &= ~(0x0FUL << shift); /* Xóa 4 bit cũ */
        port->CRL |= ((pin_val & 0x0FUL) << shift);
    } else {
        uint8_t shift = (pin - 8) * 4;
        port->CRH &= ~(0x0FUL << shift); /* Xóa 4 bit cũ */
        port->CRH |= ((pin_val & 0x0FUL) << shift);
    }

    return STATUS_OK;
}

void gpio_write_pin(gpio_register_t *port, uint8_t pin, gpio_pin_state_t state) {
    if (port == NULL || pin > 15) {
        return;
    }
    if (state == GPIO_PIN_SET) {
        port->BSRR = (1UL << pin);
    } else {
        port->BRR = (1UL << pin);
    }
}

void gpio_toggle_pin(gpio_register_t *port, uint8_t pin) {
    if (port == NULL || pin > 15) {
        return;
    }
    /* Đọc trạng thái hiện tại trong ODR để đảo */
    if (port->ODR & (1UL << pin)) {
        port->BRR = (1UL << pin);
    } else {
        port->BSRR = (1UL << pin);
    }
}

gpio_pin_state_t gpio_read_pin(const gpio_register_t *port, uint8_t pin) {
    if (port == NULL || pin > 15) {
        return GPIO_PIN_RESET;
    }
    return ((port->IDR & (1UL << pin)) != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}