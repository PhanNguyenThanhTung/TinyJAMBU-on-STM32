#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "gpio_register.h"
#include "project_types.h"

/* Trạng thái logic của chân */
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET   = 1
} gpio_pin_state_t;

/* Tốc độ ngõ ra */
typedef enum {
    GPIO_SPEED_INPUT = 0x00, /* Chế độ Input (MODE = 00) */
    GPIO_SPEED_10MHZ = 0x01, /* Output 10MHz (MODE = 01) */
    GPIO_SPEED_2MHZ  = 0x02, /* Output 2MHz (MODE = 10) */
    GPIO_SPEED_50MHZ = 0x03  /* Output 50MHz (MODE = 11) */
} gpio_speed_t;

/* Chế độ hoạt động */
typedef enum {
    /* Input */
    GPIO_MODE_INPUT_ANALOG    = 0x00,
    GPIO_MODE_INPUT_FLOATING  = 0x04,
    GPIO_MODE_INPUT_PULLDOWN  = 0x08,
    GPIO_MODE_INPUT_PULLUP    = 0x09,

    /* Output */
    GPIO_MODE_OUTPUT_PUSHPULL = 0x00,
    GPIO_MODE_OUTPUT_OPENDRAIN= 0x04,
    GPIO_MODE_AF_PUSHPULL     = 0x08, 
    GPIO_MODE_AF_OPENDRAIN    = 0x0C
} gpio_mode_t;

/* Pin config */
typedef struct {
    uint8_t pin;        /* Số chân: từ 0 đến 15 */
    gpio_mode_t mode;   /* Chế độ I/O */
    gpio_speed_t speed; /* Tốc độ */
} gpio_config_t;

/* Hàm điểu khiển pin GPIO */
status_t gpio_init_pin(gpio_register_t *port, const gpio_config_t *config);
void gpio_write_pin(gpio_register_t *port, uint8_t pin, gpio_pin_state_t state);
void gpio_toggle_pin(gpio_register_t *port, uint8_t pin);
gpio_pin_state_t gpio_read_pin(const gpio_register_t *port, uint8_t pin);

#endif /* GPIO_H */