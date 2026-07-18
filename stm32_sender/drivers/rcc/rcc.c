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
    rcc_enable_hse();
    rcc_configure_flash();
    rcc_configure_bus_prescalers();
    rcc_configure_pll();
    rcc_enable_pll();
    rcc_switch_system_clock_to_pll();

    system_core_clock_hz = 36000000UL;
    return STATUS_OK;
}

static void rcc_configure_flash(void);