#ifndef RCC_H
#define RCC_H

#include<stdint.h>
#include "project_types.h"
/*rcc peripheral function*/
status_t rcc_init(void);
void rcc_enable_gpioa_clock(void);
void rcc_enable_gpiob_clock(void);
void rcc_enable_gpioc_clock(void);
void rcc_enable_usart1_clock(void);

extern uint32_t system_core_clock_hz;

#endif