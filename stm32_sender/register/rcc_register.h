#ifndef RCC_REGISTER_H
#define RCC_REGISTER_H

#include <stdint.h>
#include "memory_map.h"

volatile uint32_t CR;

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} rcc_register_t;

#define RCC ((rcc_register_t *)RCC_BASE)

#endif