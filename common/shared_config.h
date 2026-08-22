#ifndef SHARED_CONFIG_H
#define SHARED_CONFIG_H

#include <stdint.h>

/* Demonstration key shared by the STM32 sender and ESP32 receiver.
 * Replace this value and provision it securely before a real deployment. */
static const uint8_t PROJECT_SHARED_KEY[16] = {
    0x00U, 0x01U, 0x02U, 0x03U,
    0x04U, 0x05U, 0x06U, 0x07U,
    0x08U, 0x09U, 0x0AU, 0x0BU,
    0x0CU, 0x0DU, 0x0EU, 0x0FU
};

#define PROJECT_DEVICE_ID              0x103C8E01UL
#define PROJECT_NONCE_MAGIC            0x544A4D42UL

#endif /* SHARED_CONFIG_H */
