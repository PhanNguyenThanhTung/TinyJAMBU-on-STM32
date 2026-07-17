#ifndef IWDG_REGISTER_H
#define IWDG_REGISTER_H

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    volatile uint32_t KR;
    volatile uint32_t PR;
    volatile uint32_t RLR;
    volatile uint32_t SR;
} iwdg_register_t;

#define IWDG ((iwdg_register_t *)IWDG_BASE)

#endif