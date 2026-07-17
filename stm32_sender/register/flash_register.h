#ifndef FLASH_REGISTER_H
#define FLASH_REGISTER_H

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} flash_register_t;

#define FLASH ((flash_register_t *)FLASH_BASE)

#endif