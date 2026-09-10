#include "app.h"

#include <stdint.h>

#include "encryption.h"
#include "gpio.h"
#include "gpio_register.h"
#include "project_config.h"
#include "protocol_config.h"
#include "rcc.h"
#include "systick.h"
#include "uart.h"
#include "watchdog.h"

#define STATUS_LED_PIN                13U
#define TELEMETRY_FORMAT_VERSION      1U
#define TELEMETRY_PAYLOAD_SIZE        10U

static uint32_t g_sequence_number;

static void store_uint32_be(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)(value >> 24);
    destination[1] = (uint8_t)(value >> 16);
    destination[2] = (uint8_t)(value >> 8);
    destination[3] = (uint8_t)value;
}

static void build_telemetry_payload(uint8_t payload[TELEMETRY_PAYLOAD_SIZE])
{
    payload[0] = TELEMETRY_FORMAT_VERSION;
    payload[1] = 0U; /* Reserved status flags. */
    store_uint32_be(payload + 2U, g_sequence_number);
    store_uint32_be(payload + 6U, systick_get_tick());
}

status_t app_init(void)
{
    static const gpio_config_t led_config = {
        .pin = STATUS_LED_PIN,
        .mode = GPIO_MODE_OUTPUT_PUSHPULL,
        .speed = GPIO_SPEED_2MHZ
    };
    status_t status;

    status = rcc_init();
    if (status != STATUS_OK) {
        return status;
    }

    rcc_enable_gpioc_clock();
    status = gpio_init_pin(GPIOC, &led_config);
    if (status != STATUS_OK) {
        return status;
    }
    gpio_write_pin(GPIOC, STATUS_LED_PIN, GPIO_PIN_SET); /* Blue Pill LED is active-low. */

    status = systick_init();
    if (status != STATUS_OK) {
        return status;
    }
    status = uart_init(UART_BAUD_RATE);
    if (status != STATUS_OK) {
        return status;
    }
    status = encryption_init();
    if (status != STATUS_OK) {
        return status;
    }
    status = watchdog_init(WATCHDOG_TIMEOUT_MS);
    if (status != STATUS_OK) {
        return status;
    }

    g_sequence_number = 0U;
    return STATUS_OK;
}

void app_run(void)
{
    uint8_t payload[TELEMETRY_PAYLOAD_SIZE];
    uint8_t frame[PROTOCOL_MAX_FRAME_SIZE];
    uint32_t last_send_tick = systick_get_tick() - DATA_SEND_PERIOD_MS;

    for (;;) {
        const uint32_t now = systick_get_tick();

        if ((now - last_send_tick) >= DATA_SEND_PERIOD_MS) {
            uint16_t frame_length = 0U;
            status_t status;

            last_send_tick = now;
            build_telemetry_payload(payload);
            status = encryption_build_frame(
                FRAME_TYPE_DATA,
                payload,
                sizeof(payload),
                frame,
                sizeof(frame),
                &frame_length);
            if (status == STATUS_OK) {
                uart_send_buffer(frame, frame_length);
                gpio_toggle_pin(GPIOC, STATUS_LED_PIN);
                ++g_sequence_number;
            }
        }

        watchdog_refresh();
    }
}
