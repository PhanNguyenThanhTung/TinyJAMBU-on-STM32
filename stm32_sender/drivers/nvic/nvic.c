#include "nvic.h"

#include "nvic_register.h"

/* STM32F103C8T6 medium-density vector table exposes IRQ0 through IRQ42. */
#define NVIC_IRQ_COUNT                43U
#define NVIC_PRIORITY_LEVEL_COUNT     16U

status_t nvic_enable_irq(uint8_t irq_number)
{
    if (irq_number >= NVIC_IRQ_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }
    NVIC->ISER[irq_number / 32U] = 1UL << (irq_number % 32U);
    return STATUS_OK;
}

status_t nvic_disable_irq(uint8_t irq_number)
{
    if (irq_number >= NVIC_IRQ_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }
    NVIC->ICER[irq_number / 32U] = 1UL << (irq_number % 32U);
    return STATUS_OK;
}

status_t nvic_clear_pending_irq(uint8_t irq_number)
{
    if (irq_number >= NVIC_IRQ_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }
    NVIC->ICPR[irq_number / 32U] = 1UL << (irq_number % 32U);
    return STATUS_OK;
}

status_t nvic_set_priority(uint8_t irq_number, uint8_t priority)
{
    volatile uint8_t *priority_registers = (volatile uint8_t *)NVIC->IPR;

    if ((irq_number >= NVIC_IRQ_COUNT) ||
        (priority >= NVIC_PRIORITY_LEVEL_COUNT)) {
        return STATUS_INVALID_PARAMETER;
    }

    /* STM32F103 implements the upper four priority bits. */
    priority_registers[irq_number] = (uint8_t)(priority << 4);
    return STATUS_OK;
}
