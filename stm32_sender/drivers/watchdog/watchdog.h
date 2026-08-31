#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

#include "project_types.h"

status_t watchdog_init(uint32_t timeout_ms);
void watchdog_refresh(void);

#endif /* WATCHDOG_H */
