#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stdint.h>

#include "project_types.h"

status_t encryption_init(void);
status_t encryption_build_frame(
    uint8_t message_type,
    const uint8_t *plaintext,
    uint16_t plaintext_length,
    uint8_t *frame_out,
    uint16_t frame_capacity,
    uint16_t *frame_length_out);

#endif /* ENCRYPTION_H */
