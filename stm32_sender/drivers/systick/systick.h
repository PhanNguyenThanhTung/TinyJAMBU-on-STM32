#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include "project_types.h" /* Lay kieu enum status_t */

status_t systick_init(void);
void systick_delay_ms(uint32_t delay_ms);
uint32_t systick_get_tick(void);

#endif