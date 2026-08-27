#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>

#include "project_types.h"

status_t nvic_enable_irq(uint8_t irq_number);
status_t nvic_disable_irq(uint8_t irq_number);
status_t nvic_clear_pending_irq(uint8_t irq_number);
status_t nvic_set_priority(uint8_t irq_number, uint8_t priority);

#endif /* NVIC_H */
