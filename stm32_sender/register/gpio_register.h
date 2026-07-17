#ifndef GPIO_REGISTER_H
#define GPIO_REGISTER_H

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} gpio_register_t;

#define GPIOA ((gpio_register_t *)GPIOA_BASE)
#define GPIOB ((gpio_register_t *)GPIOB_BASE)
#define GPIOC ((gpio_register_t *)GPIOC_BASE)

#endif
