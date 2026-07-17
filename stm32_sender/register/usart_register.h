#ifndef USART_REGISTER_H
#define USART_REGISTER_H

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} usart_register_t;

#define USART1 ((usart_register_t *)USART1_BASE)

#endif