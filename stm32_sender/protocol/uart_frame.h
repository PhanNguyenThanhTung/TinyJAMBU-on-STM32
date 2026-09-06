#ifndef UART_FRAME_H
#define UART_FRAME_H

#include <stdint.h>

#include "project_types.h"
#include "protocol_config.h"

status_t uart_frame_pack(
    const uint8_t *key,
    const uint8_t *nonce,
    frame_type_t message_type,
    const uint8_t *plaintext,
    uint16_t plaintext_length,
    uint8_t *frame_out,
    uint16_t frame_capacity,
    uint16_t *frame_length_out);

status_t uart_frame_unpack(
    const uint8_t *key,
    const uint8_t *frame,
    uint16_t frame_length,
    frame_type_t *message_type_out,
    uint8_t *plaintext_out,
    uint16_t plaintext_capacity,
    uint16_t *plaintext_length_out);

#endif /* UART_FRAME_H */
