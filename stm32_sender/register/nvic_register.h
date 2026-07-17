#ifndef NVIC_REGISTER_H
#define NVIC_R

#include <stdint.h>
#include "memory_map.h"

typedef struct {
    /* Interrupt Set-Enable Registers (Offset: 0x000 - 0x008) */
    volatile uint32_t ISER[3];
    /* Reserved space (Offset: 0x00C - 0x07C) */
    volatile uint32_t RESERVED0[29];
    /* Interrupt Clear-Enable Registers (Offset: 0x080 - 0x088) */
    volatile uint32_t ICER[3];
    /* Reserved space (Offset: 0x08C - 0x0FC) */
    volatile uint32_t RESERVED1[29];
    /* Interrupt Set-Pending Registers (Offset: 0x100 - 0x108) */
    volatile uint32_t ISPR[3];
    /* Reserved space (Offset: 0x10C - 0x17C) */
    volatile uint32_t RESERVED2[29];
    /* Interrupt Clear-Pending Registers (Offset: 0x180 - 0x188) */
    volatile uint32_t ICPR[3];
    /* Reserved space (Offset: 0x18C - 0x1FC) */
    volatile uint32_t RESERVED3[29];
    /* Interrupt Active Bit Registers (Offset: 0x200 - 0x208) */
    volatile uint32_t IABR[3];
    /* Reserved space (Offset: 0x20C - 0x2FC) */
    volatile uint32_t RESERVED4[61];
    /* Interrupt Priority Registers (Offset: 0x300 - 0x353) */
    volatile uint32_t IPR[21];   
} nvic_register_t;

#define NVIC ((nvic_register_t *)NVIC_BASE)

#endif