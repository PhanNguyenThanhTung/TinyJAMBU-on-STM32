#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

/* Peripheral Base Addresses */ 
#define RCC_BASE          0x40021000UL
#define FLASH_BASE        0x40022000UL
#define GPIOA_BASE        0x40010800UL
#define GPIOB_BASE        0x40010C00UL
#define GPIOC_BASE        0x40011000UL
#define USART1_BASE       0x40013800UL
#define IWDG_BASE         0x40003000UL
/* Cortex-M3 Core Peripheral Base Addresses */
#define SYSTICK_BASE      0xE000E010UL
#define NVIC_BASE         0xE000E100UL

#endif