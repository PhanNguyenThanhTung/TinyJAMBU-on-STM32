#ifndef RCC_H
#define RCC_H

#include<stdint.h>
#include "project_types.h"

status_t rcc_init(void);

uint32_t rcc_get_system_clock_hz(void);

#endif