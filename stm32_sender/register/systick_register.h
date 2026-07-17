#ifndef SYSTICK_REGISTER_H
#define SYSTICK_REGISTER_H

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} systick_register_t;

#define SYSTICK ((systick_register_t *)SYSTICK_BASE)

#endif