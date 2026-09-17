#include <stdio.h>
#include <string.h>

#include "crc16.h"
#include "uart_frame.h"

static int failures = 0;

#define CHECK(condition, message)                                                \
    do {                                                                         \
        if (!(condition)) {                                                      \
            fprintf(stderr, "FAIL: %s (line %d)\n", (message), __LINE__);       \
            ++failures;                                                          \
        }                                                                        \
    } while (0)

static const uint8_t key[KEY_SIZE_BYTES] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};
static const uint8_t nonce[NONCE_SIZE_BYTES] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B
};

static void refresh_crc(uint8_t *frame, uint16_t frame_length)
{
    const uint16_t data_length = (uint16_t)(frame_length - PROTOCOL_CRC_SIZE);
    const uint16_t crc = crc16_calculate(frame, data_length);

    frame[data_length] = (uint8_t)(crc >> 8);
    frame[data_length + 1U] = (uint8_t)crc;
}

static void test_round_trip(void)
{
    static const uint8_t payload[] = "STM32 TinyJAMBU telemetry";
    uint8_t frame[PROTOCOL_MAX_FRAME_SIZE];
    uint8_t decoded[MAX_PAYLOAD_SIZE];
    uint16_t frame_length = 0U;
    uint16_t decoded_length = 0U;
    frame_type_t type = FRAME_TYPE_ACK;
    status_t status;

    status = uart_frame_pack(
        key, nonce, FRAME_TYPE_DATA, payload, sizeof(payload) - 1U,
        frame, sizeof(frame), &frame_length);
    CHECK(status == STATUS_OK, "frame pack status");
    CHECK(frame_length == PROTOCOL_OVERHEAD_SIZE + sizeof(payload) - 1U,
          "frame length");

    status = uart_frame_unpack(
        key, frame, frame_length, &type, decoded, sizeof(decoded), &decoded_length);
    CHECK(status == STATUS_OK, "frame unpack status");
    CHECK(type == FRAME_TYPE_DATA, "message type round-trip");
    CHECK(decoded_length == sizeof(payload) - 1U, "payload length round-trip");
    CHECK(memcmp(decoded, payload, decoded_length) == 0, "payload round-trip");
}

static void test_validation(void)
{
    static const uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t frame[PROTOCOL_MAX_FRAME_SIZE];
    uint8_t decoded[MAX_PAYLOAD_SIZE];
    uint16_t frame_length = 0U;
    uint16_t decoded_length = 0U;
    frame_type_t type;
    status_t status;

    status = uart_frame_pack(
        key, nonce, FRAME_TYPE_ALERT, payload, sizeof(payload),
        frame, PROTOCOL_OVERHEAD_SIZE, &frame_length);
    CHECK(status == STATUS_BUFFER_OVERFLOW, "small frame buffer rejected");

    status = uart_frame_pack(
        key, nonce, FRAME_TYPE_ALERT, payload, sizeof(payload),
        frame, sizeof(frame), &frame_length);
    CHECK(status == STATUS_OK, "validation fixture packed");

    frame[0] ^= 0x01U;
    status = uart_frame_unpack(
        key, frame, frame_length, &type, decoded, sizeof(decoded), &decoded_length);
    CHECK(status == STATUS_ERROR, "invalid start byte rejected");
    frame[0] ^= 0x01U;

    frame[PROTOCOL_HEADER_SIZE + PROTOCOL_NONCE_SIZE] ^= 0x01U;
    status = uart_frame_unpack(
        key, frame, frame_length, &type, decoded, sizeof(decoded), &decoded_length);
    CHECK(status == STATUS_ERROR, "CRC corruption rejected");
    frame[PROTOCOL_HEADER_SIZE + PROTOCOL_NONCE_SIZE] ^= 0x01U;

    frame[PROTOCOL_HEADER_SIZE + PROTOCOL_NONCE_SIZE] ^= 0x01U;
    refresh_crc(frame, frame_length);
    memset(decoded, 0xA5, sizeof(payload));
    status = uart_frame_unpack(
        key, frame, frame_length, &type, decoded, sizeof(decoded), &decoded_length);
    CHECK(status == STATUS_AUTHENTICATION_FAILED,
          "ciphertext tampering with corrected CRC rejected by AEAD");
    for (size_t index = 0U; index < sizeof(payload); ++index) {
        CHECK(decoded[index] == 0U, "failed frame clears plaintext");
    }
}

int main(void)
{
    test_round_trip();
    test_validation();

    if (failures == 0) {
        puts("UART frame tests: PASS");
        return 0;
    }
    fprintf(stderr, "UART frame tests: %d failure(s)\n", failures);
    return 1;
}
