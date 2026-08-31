#include "watchdog.h"

#include "iwdg_register.h"

#define IWDG_LSI_FREQUENCY_HZ         40000UL
#define IWDG_WRITE_ACCESS_ENABLE      0x5555UL
#define IWDG_RELOAD_KEY               0xAAAAUL
#define IWDG_START_KEY                0xCCCCUL
#define IWDG_STATUS_UPDATE_MASK       0x03UL
#define IWDG_UPDATE_TIMEOUT           100000UL

status_t watchdog_init(uint32_t timeout_ms)
{
    static const uint16_t divisors[] = {4U, 8U, 16U, 32U, 64U, 128U, 256U};
    uint32_t reload = 0U;
    uint32_t prescaler_code = 0U;
    uint32_t timeout = IWDG_UPDATE_TIMEOUT;
    bool found = false;

    if (timeout_ms == 0U) {
        return STATUS_INVALID_PARAMETER;
    }

    for (uint32_t index = 0U; index < 7U; ++index) {
        const uint64_t ticks =
            ((uint64_t)timeout_ms * IWDG_LSI_FREQUENCY_HZ +
             ((uint64_t)divisors[index] * 1000U) - 1U) /
            ((uint64_t)divisors[index] * 1000U);

        if ((ticks >= 1U) && (ticks <= 4096U)) {
            reload = (uint32_t)ticks - 1U;
            prescaler_code = index;
            found = true;
            break;
        }
    }
    if (!found) {
        return STATUS_INVALID_PARAMETER;
    }

    IWDG->KR = IWDG_WRITE_ACCESS_ENABLE;
    IWDG->PR = prescaler_code;
    IWDG->RLR = reload;
    while ((IWDG->SR & IWDG_STATUS_UPDATE_MASK) != 0U) {
        if (timeout-- == 0U) {
            return STATUS_TIMEOUT;
        }
    }

    IWDG->KR = IWDG_RELOAD_KEY;
    IWDG->KR = IWDG_START_KEY;
    return STATUS_OK;
}

void watchdog_refresh(void)
{
    IWDG->KR = IWDG_RELOAD_KEY;
}
