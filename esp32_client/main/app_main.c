#include "app_main.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "shared_config.h"
#include "uart_frame.h"

#define TELEMETRY_PAYLOAD_SIZE        10U
#define TELEMETRY_FORMAT_VERSION      1U

static const char *TAG = "tinyjambu_rx";

static uint8_t g_frame[PROTOCOL_MAX_FRAME_SIZE];
static uint16_t g_frame_length;
static uint16_t g_expected_length;
static bool g_replay_state_valid;
static uint32_t g_last_epoch;
static uint32_t g_last_counter;

static uint32_t load_uint32_be(const uint8_t *source)
{
    return ((uint32_t)source[0] << 24) |
           ((uint32_t)source[1] << 16) |
           ((uint32_t)source[2] << 8) |
           (uint32_t)source[3];
}

static bool nonce_is_fresh(const uint8_t *nonce)
{
    const uint32_t device_id = load_uint32_be(nonce);
    const uint32_t epoch = load_uint32_be(nonce + 4U);
    const uint32_t counter = load_uint32_be(nonce + 8U);

    if (device_id != PROJECT_DEVICE_ID) {
        ESP_LOGW(TAG, "Rejected frame from unknown device 0x%08" PRIX32, device_id);
        return false;
    }
    if (g_replay_state_valid &&
        ((epoch < g_last_epoch) ||
         ((epoch == g_last_epoch) && (counter <= g_last_counter)))) {
        ESP_LOGW(TAG, "Rejected replay epoch=%" PRIu32 " counter=%" PRIu32,
                 epoch, counter);
        return false;
    }

    g_last_epoch = epoch;
    g_last_counter = counter;
    g_replay_state_valid = true;
    return true;
}

static void handle_telemetry(const uint8_t *payload, uint16_t length)
{
    if ((length != TELEMETRY_PAYLOAD_SIZE) ||
        (payload[0] != TELEMETRY_FORMAT_VERSION)) {
        ESP_LOGW(TAG, "Unsupported telemetry format (length=%u, version=%u)",
                 (unsigned)length, (unsigned)payload[0]);
        return;
    }

    ESP_LOGI(TAG,
             "Telemetry flags=0x%02X sequence=%" PRIu32 " uptime_ms=%" PRIu32,
             payload[1], load_uint32_be(payload + 2U),
             load_uint32_be(payload + 6U));
}

static void handle_complete_frame(void)
{
    uint8_t plaintext[MAX_PAYLOAD_SIZE];
    uint16_t plaintext_length = 0U;
    frame_type_t message_type;
    status_t status = uart_frame_unpack(
        PROJECT_SHARED_KEY,
        g_frame,
        g_frame_length,
        &message_type,
        plaintext,
        sizeof(plaintext),
        &plaintext_length);

    if (status != STATUS_OK) {
        ESP_LOGW(TAG, "Rejected frame, status=%d", (int)status);
        return;
    }
    if (!nonce_is_fresh(g_frame + PROTOCOL_HEADER_SIZE)) {
        return;
    }

    switch (message_type) {
        case FRAME_TYPE_DATA:
            handle_telemetry(plaintext, plaintext_length);
            break;
        case FRAME_TYPE_ALERT:
            ESP_LOGW(TAG, "Authenticated alert received (%u bytes)",
                    (unsigned)plaintext_length);
            ESP_LOG_BUFFER_HEX_LEVEL(TAG, plaintext, plaintext_length, ESP_LOG_WARN);
            break;
        case FRAME_TYPE_ACK:
            ESP_LOGI(TAG, "Authenticated ACK received (%u bytes)",
                    (unsigned)plaintext_length);
            break;
        default:
            break;
    }
}

static void parser_reset(void)
{
    g_frame_length = 0U;
    g_expected_length = 0U;
}

static void parser_consume(uint8_t byte)
{
    if (g_frame_length == 0U) {
        if (byte == PROTOCOL_START_BYTE) {
            g_frame[g_frame_length++] = byte;
        }
        return;
    }

    g_frame[g_frame_length++] = byte;
    if (g_frame_length == 3U) {
        const uint16_t payload_length =
            ((uint16_t)g_frame[1] << 8) | g_frame[2];

        if (payload_length > MAX_PAYLOAD_SIZE) {
            parser_reset();
            if (byte == PROTOCOL_START_BYTE) {
                g_frame[g_frame_length++] = byte;
            }
            return;
        }
        g_expected_length = (uint16_t)(PROTOCOL_OVERHEAD_SIZE + payload_length);
    }

    if ((g_expected_length > 0U) && (g_frame_length == g_expected_length)) {
        handle_complete_frame();
        parser_reset();
    } else if (g_frame_length >= sizeof(g_frame)) {
        parser_reset();
    }
}

void app_main(void)
{
    static const uart_config_t uart_config = {
        .baud_rate = RECEIVER_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };
    uint8_t receive_buffer[64];

    ESP_ERROR_CHECK(uart_driver_install(
        RECEIVER_UART_PORT, RECEIVER_UART_BUFFER_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(RECEIVER_UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(
        RECEIVER_UART_PORT,
        RECEIVER_UART_TX_PIN,
        RECEIVER_UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE));

    parser_reset();
    ESP_LOGI(TAG,
             "TinyJAMBU UART receiver ready: UART%d RX=%d TX=%d baud=%d",
             RECEIVER_UART_PORT, RECEIVER_UART_RX_PIN,
             RECEIVER_UART_TX_PIN, RECEIVER_UART_BAUD_RATE);

    for (;;) {
        const int received = uart_read_bytes(
            RECEIVER_UART_PORT,
            receive_buffer,
            sizeof(receive_buffer),
            pdMS_TO_TICKS(100));

        for (int index = 0; index < received; ++index) {
            parser_consume(receive_buffer[index]);
        }
    }
}
