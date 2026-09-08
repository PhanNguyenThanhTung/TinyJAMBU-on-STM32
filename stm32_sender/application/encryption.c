#include "encryption.h"

#include <stdbool.h>
#include <stddef.h>

#include "flash_register.h"
#include "project_config.h"
#include "shared_config.h"
#include "uart_frame.h"

/* Two flash pages are reserved by the linker for a power-fail-safe epoch log. */
extern uint8_t __nonce_storage_start__[];

#define FLASH_KEY1                    0x45670123UL
#define FLASH_KEY2                    0xCDEF89ABUL
#define FLASH_SR_BSY                  (1UL << 0)
#define FLASH_SR_PGERR                (1UL << 2)
#define FLASH_SR_WRPRTERR             (1UL << 4)
#define FLASH_SR_EOP                  (1UL << 5)
#define FLASH_CR_PG                   (1UL << 0)
#define FLASH_CR_PER                  (1UL << 1)
#define FLASH_CR_STRT                 (1UL << 6)
#define FLASH_CR_LOCK                 (1UL << 7)
#define FLASH_OPERATION_TIMEOUT       1000000UL
#define FLASH_PAGE_SIZE_BYTES         1024U
#define NONCE_PAGE_COUNT              2U
#define NONCE_RECORD_SIZE_BYTES       8U
#define NONCE_RECORDS_PER_PAGE        (FLASH_PAGE_SIZE_BYTES / NONCE_RECORD_SIZE_BYTES)
#define ERASED_FLASH_WORD             0xFFFFFFFFUL

typedef struct {
    uint32_t epoch;
    uint32_t guard;
} nonce_record_t;

typedef struct {
    bool has_valid_record;
    uint32_t maximum_epoch;
    uint32_t maximum_slot;
} nonce_page_state_t;

_Static_assert(sizeof(nonce_record_t) == NONCE_RECORD_SIZE_BYTES,
               "nonce journal record size changed");
_Static_assert(NONCE_SIZE_BYTES == 12U, "nonce layout requires 12 bytes");
_Static_assert(sizeof(PROJECT_SHARED_KEY) == KEY_SIZE_BYTES,
               "shared key size must match TinyJAMBU-128");

static uint32_t g_nonce_epoch;
static uint32_t g_message_counter;
static bool g_initialized;

static uint32_t nonce_page_address(uint32_t page_index)
{
    return (uint32_t)__nonce_storage_start__ +
           (page_index * FLASH_PAGE_SIZE_BYTES);
}

static bool nonce_record_is_valid(volatile const nonce_record_t *record)
{
    return record->guard == (record->epoch ^ PROJECT_NONCE_MAGIC);
}

static bool nonce_record_is_erased(volatile const nonce_record_t *record)
{
    return (record->epoch == ERASED_FLASH_WORD) &&
           (record->guard == ERASED_FLASH_WORD);
}

static nonce_page_state_t nonce_page_scan(uint32_t page_index)
{
    volatile const nonce_record_t *records =
        (volatile const nonce_record_t *)nonce_page_address(page_index);
    nonce_page_state_t state = {false, 0U, 0U};

    for (uint32_t slot = 0U; slot < NONCE_RECORDS_PER_PAGE; ++slot) {
        if (nonce_record_is_valid(&records[slot]) &&
            (!state.has_valid_record ||
             (records[slot].epoch > state.maximum_epoch))) {
            state.has_valid_record = true;
            state.maximum_epoch = records[slot].epoch;
            state.maximum_slot = slot;
        }
    }
    return state;
}

static int32_t nonce_page_find_erased_slot(uint32_t page_index, uint32_t start_slot)
{
    volatile const nonce_record_t *records =
        (volatile const nonce_record_t *)nonce_page_address(page_index);

    for (uint32_t slot = start_slot; slot < NONCE_RECORDS_PER_PAGE; ++slot) {
        if (nonce_record_is_erased(&records[slot])) {
            return (int32_t)slot;
        }
    }
    return -1;
}

static status_t flash_wait_ready(void)
{
    uint32_t timeout = FLASH_OPERATION_TIMEOUT;

    while ((FLASH->SR & FLASH_SR_BSY) != 0U) {
        if (timeout-- == 0U) {
            return STATUS_TIMEOUT;
        }
    }
    if ((FLASH->SR & (FLASH_SR_PGERR | FLASH_SR_WRPRTERR)) != 0U) {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

static void flash_clear_status(void)
{
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR;
}

static status_t flash_unlock(void)
{
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    return ((FLASH->CR & FLASH_CR_LOCK) == 0U) ? STATUS_OK : STATUS_ERROR;
}

static status_t flash_erase_page(uint32_t page_index)
{
    status_t status;

    flash_clear_status();
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = nonce_page_address(page_index);
    FLASH->CR |= FLASH_CR_STRT;
    status = flash_wait_ready();
    FLASH->CR &= ~FLASH_CR_PER;
    return status;
}

static status_t flash_program_halfword(uint32_t address, uint16_t value)
{
    status_t status;

    flash_clear_status();
    FLASH->CR |= FLASH_CR_PG;
    *(volatile uint16_t *)address = value;
    status = flash_wait_ready();
    FLASH->CR &= ~FLASH_CR_PG;
    if (status != STATUS_OK) {
        return status;
    }
    return (*(volatile uint16_t *)address == value) ? STATUS_OK : STATUS_ERROR;
}

static status_t nonce_record_program(
    uint32_t page_index,
    uint32_t slot,
    uint32_t epoch)
{
    const uint32_t address = nonce_page_address(page_index) +
                             (slot * sizeof(nonce_record_t));
    const uint32_t words[2] = {epoch, epoch ^ PROJECT_NONCE_MAGIC};

    for (uint32_t word = 0U; word < 2U; ++word) {
        status_t status = flash_program_halfword(
            address + (word * 4U), (uint16_t)words[word]);
        if (status == STATUS_OK) {
            status = flash_program_halfword(
                address + (word * 4U) + 2U, (uint16_t)(words[word] >> 16));
        }
        if (status != STATUS_OK) {
            return status;
        }
    }

    return nonce_record_is_valid((volatile const nonce_record_t *)address)
               ? STATUS_OK
               : STATUS_ERROR;
}

static status_t nonce_reserve_epoch(uint32_t *epoch_out)
{
    nonce_page_state_t pages[NONCE_PAGE_COUNT];
    uint32_t active_page = 0U;
    uint32_t next_epoch = 0U;
    int32_t target_slot;
    status_t status = flash_unlock();

    if ((epoch_out == NULL) || (status != STATUS_OK)) {
        return (epoch_out == NULL) ? STATUS_INVALID_PARAMETER : status;
    }

    for (uint32_t page = 0U; page < NONCE_PAGE_COUNT; ++page) {
        pages[page] = nonce_page_scan(page);
        if (pages[page].has_valid_record &&
            (!pages[active_page].has_valid_record ||
             (pages[page].maximum_epoch > pages[active_page].maximum_epoch))) {
            active_page = page;
        }
    }

    if (pages[active_page].has_valid_record) {
        if (pages[active_page].maximum_epoch == UINT32_MAX) {
            FLASH->CR |= FLASH_CR_LOCK;
            return STATUS_ERROR;
        }
        next_epoch = pages[active_page].maximum_epoch + 1U;
        target_slot = nonce_page_find_erased_slot(
            active_page, pages[active_page].maximum_slot + 1U);
    } else {
        target_slot = nonce_page_find_erased_slot(active_page, 0U);
    }

    if (target_slot < 0) {
        const uint32_t old_page = active_page;
        active_page ^= 1U;
        status = flash_erase_page(active_page);
        if (status == STATUS_OK) {
            status = nonce_record_program(active_page, 0U, next_epoch);
        }
        if (status == STATUS_OK) {
            /* The new epoch is durable before the old page is reclaimed. */
            (void)flash_erase_page(old_page);
        }
    } else {
        status = nonce_record_program(active_page, (uint32_t)target_slot, next_epoch);
    }

    FLASH->CR |= FLASH_CR_LOCK;
    if (status == STATUS_OK) {
        *epoch_out = next_epoch;
    }
    return status;
}

static void store_uint32_be(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)(value >> 24);
    destination[1] = (uint8_t)(value >> 16);
    destination[2] = (uint8_t)(value >> 8);
    destination[3] = (uint8_t)value;
}

status_t encryption_init(void)
{
    status_t status = nonce_reserve_epoch(&g_nonce_epoch);

    if (status != STATUS_OK) {
        return status;
    }
    g_message_counter = 0U;
    g_initialized = true;
    return STATUS_OK;
}

status_t encryption_build_frame(
    uint8_t message_type,
    const uint8_t *plaintext,
    uint16_t plaintext_length,
    uint8_t *frame_out,
    uint16_t frame_capacity,
    uint16_t *frame_length_out)
{
    uint8_t nonce[NONCE_SIZE_BYTES];
    status_t status;

    if (!g_initialized) {
        return STATUS_ERROR;
    }

    store_uint32_be(nonce, PROJECT_DEVICE_ID);
    store_uint32_be(nonce + 4U, g_nonce_epoch);
    store_uint32_be(nonce + 8U, g_message_counter);

    status = uart_frame_pack(
        PROJECT_SHARED_KEY,
        nonce,
        (frame_type_t)message_type,
        plaintext,
        plaintext_length,
        frame_out,
        frame_capacity,
        frame_length_out);
    if (status == STATUS_OK) {
        if (g_message_counter == UINT32_MAX) {
            g_initialized = false;
        } else {
            ++g_message_counter;
        }
    }
    return status;
}
