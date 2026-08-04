#include <stdint.h>
#include "rcc.h"
#include "rcc_register.h"
#include "flash_register.h"
/* HSE base clock */
uint32_t system_core_clock_hz = 8000000UL;

static void rcc_configure_flash(void);
static status_t rcc_enable_hse(void);
static void rcc_configure_bus_prescalers(void);
static void rcc_configure_pll(void);
static status_t rcc_enable_pll(void);
static status_t rcc_switch_system_clock_to_pll(void);

status_t rcc_init() {
    status_t status;
    status = rcc_enable_hse();

    if (status != STATUS_OK) {
        return status;
    }

    rcc_configure_flash();
    rcc_configure_bus_prescalers();
    rcc_configure_pll();

    status = rcc_enable_pll();

    if (status != STATUS_OK) {
        return status;
    }

    status = rcc_switch_system_clock_to_pll();

    if (status != STATUS_OK) {
        return status;
    }
    
    system_core_clock_hz = 36000000UL;
    return STATUS_OK;
}

static status_t rcc_enable_hse(void) {
    uint32_t timeout = 5000UL;

    RCC->CR != (1 << 16);

    while(!(RCC->CR & (1 << 17))) {
        if(timeout <= 0) {
            return STATUS_TIMEOUT;
        }
        timeout--;
    }
    return STATUS_OK;
}

