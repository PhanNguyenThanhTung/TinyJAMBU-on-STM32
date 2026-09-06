#include "uart_frame.h"

#include <stddef.h>
#include <string.h>

#include "crc16.h"
#include "tinyjambu.h"

_Static_assert(PROTOCOL_NONCE_SIZE == TINYJAMBU_NONCE_SIZE_BYTES,
               "protocol nonce size must match TinyJAMBU");
_Static_assert(PROTOCOL_TAG_SIZE == TINYJAMBU_TAG_SIZE_BYTES,
               "protocol tag size must match TinyJAMBU");

static bool frame_type_is_valid(frame_type_t message_type)
{
    return (message_type == FRAME_TYPE_DATA) ||
           (message_type == FRAME_TYPE_ALERT) ||
           (message_type == FRAME_TYPE_ACK);
}

status_t uart_frame_pack(
    const uint8_t *key,
    const uint8_t *nonce,
    frame_type_t message_type,
    const uint8_t *plaintext,
    uint16_t plaintext_length,
    uint8_t *frame_out,
    uint16_t frame_capacity,
    uint16_t *frame_length_out)
{
    uint8_t *ciphertext;
    uint8_t *tag;
    uint16_t authenticated_length;
    uint16_t required_length;
    uint16_t crc;
    status_t status;

    if (frame_length_out != NULL) {
        *frame_length_out = 0U;
    }
    if ((key == NULL) || (nonce == NULL) || (frame_out == NULL) ||
        (frame_length_out == NULL) || !frame_type_is_valid(message_type)) {
        return STATUS_INVALID_PARAMETER;
    }
    if ((plaintext_length > 0U) && (plaintext == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (plaintext_length > MAX_PAYLOAD_SIZE) {
        return STATUS_BUFFER_OVERFLOW;
    }

    required_length = (uint16_t)(PROTOCOL_OVERHEAD_SIZE + plaintext_length);
    if (frame_capacity < required_length) {
        return STATUS_BUFFER_OVERFLOW;
    }

    frame_out[0] = PROTOCOL_START_BYTE;
    frame_out[1] = (uint8_t)(plaintext_length >> 8);
    frame_out[2] = (uint8_t)plaintext_length;
    frame_out[3] = (uint8_t)message_type;
    memcpy(frame_out + PROTOCOL_HEADER_SIZE, nonce, PROTOCOL_NONCE_SIZE);

    ciphertext = frame_out + PROTOCOL_HEADER_SIZE + PROTOCOL_NONCE_SIZE;
    tag = ciphertext + plaintext_length;
    status = tinyjambu_encrypt(
        key,
        nonce,
        frame_out,
        PROTOCOL_HEADER_SIZE,
        plaintext,
        plaintext_length,
        ciphertext,
        tag);
    if (status != STATUS_OK) {
        return status;
    }

    authenticated_length = (uint16_t)(required_length - PROTOCOL_CRC_SIZE);
    crc = crc16_calculate(frame_out, authenticated_length);
    frame_out[authenticated_length] = (uint8_t)(crc >> 8);
    frame_out[authenticated_length + 1U] = (uint8_t)crc;
    *frame_length_out = required_length;
    return STATUS_OK;
}

status_t uart_frame_unpack(
    const uint8_t *key,
    const uint8_t *frame,
    uint16_t frame_length,
    frame_type_t *message_type_out,
    uint8_t *plaintext_out,
    uint16_t plaintext_capacity,
    uint16_t *plaintext_length_out)
{
    frame_type_t message_type;
    const uint8_t *nonce;
    const uint8_t *ciphertext;
    const uint8_t *tag;
    uint16_t payload_length;
    uint16_t authenticated_length;
    uint16_t received_crc;
    status_t status;

    if (plaintext_length_out != NULL) {
        *plaintext_length_out = 0U;
    }
    if ((key == NULL) || (frame == NULL) || (message_type_out == NULL) ||
        (plaintext_length_out == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (frame_length < PROTOCOL_OVERHEAD_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    if (frame[0] != PROTOCOL_START_BYTE) {
        return STATUS_ERROR;
    }

    payload_length = ((uint16_t)frame[1] << 8) | frame[2];
    if (payload_length > MAX_PAYLOAD_SIZE) {
        return STATUS_BUFFER_OVERFLOW;
    }
    if (frame_length != (uint16_t)(PROTOCOL_OVERHEAD_SIZE + payload_length)) {
        return STATUS_ERROR;
    }
    if ((payload_length > 0U) && (plaintext_out == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (plaintext_capacity < payload_length) {
        return STATUS_BUFFER_OVERFLOW;
    }

    message_type = (frame_type_t)frame[3];
    if (!frame_type_is_valid(message_type)) {
        return STATUS_ERROR;
    }

    authenticated_length = (uint16_t)(frame_length - PROTOCOL_CRC_SIZE);
    received_crc = ((uint16_t)frame[authenticated_length] << 8) |
                   frame[authenticated_length + 1U];
    if (!crc16_verify(frame, authenticated_length, received_crc)) {
        return STATUS_ERROR;
    }

    nonce = frame + PROTOCOL_HEADER_SIZE;
    ciphertext = nonce + PROTOCOL_NONCE_SIZE;
    tag = ciphertext + payload_length;
    status = tinyjambu_decrypt_and_verify(
        key,
        nonce,
        frame,
        PROTOCOL_HEADER_SIZE,
        ciphertext,
        payload_length,
        tag,
        plaintext_out);
    if (status != STATUS_OK) {
        return status;
    }

    *message_type_out = message_type;
    *plaintext_length_out = payload_length;
    return STATUS_OK;
}
