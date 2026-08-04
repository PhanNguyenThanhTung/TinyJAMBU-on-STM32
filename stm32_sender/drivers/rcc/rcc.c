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

    RCC->CR |= (1 << 16);

    while(!(RCC->CR & (1 << 17))) {
        if(timeout--== 0) {
            return STATUS_TIMEOUT;
        }
    }
    return STATUS_OK;
}

static void rcc_configure_flash() {
    FLASH->ACR &= ~0x07; /*Clear bit 2:*/
    FLASH->ACR |= 0x01; /*Set Latency to 1 wait state*/
    FLASH->ACR |= (1 << 4);
}

static void rcc_configure_bus_prescalers(void) {
    RCC->CFGR &= ~(0x3FF << 4);
}

static void rcc_configure_pll(void) {
    RCC->CFGR &= ~(0x3F << 16);
    RCC->CFGR |= (1 << 16);
    RCC->CFGR |= (1 << 17);
    RCC->CFGR |= (0b0111 << 18);
}

static status_t rcc_enable_pll(void) {
    uint32_t timeout = 5000UL;
    RCC->CR |= (1 << 24);

    while(!(RCC->CR & (1 << 25))) {
        if(timeout-- == 0) {
            return STATUS_TIMEOUT;
        }
    }
    return STATUS_OK;
}

static status_t rcc_switch_system_clock_to_pll(void) {
    uint32_t timeout = 5000UL;
    RCC->CFGR &= ~(0b11);
    RCC->CFGR |= 0x2; /*0b10*/

    while((RCC->CFGR & (0x3 << 2)) != (0x2 << 2)) {
        if(timeout-- == 0) {
            return STATUS_TIMEOUT;
        }
    }
    return STATUS_OK;
}

void rcc_enable_gpioa_clock(void) {
    RCC->APB2ENR |= (1 << 2);
}
void rcc_enable_gpiob_clock(void) {
    RCC->APB2ENR |= (1 << 3);
}
void rcc_enable_gpioc_clock(void) {
    RCC->APB2ENR |= (1 << 4);
}
void rcc_enable_usart1_clock(void) {
    RCC->APB2ENR |= (1 << 14);
}