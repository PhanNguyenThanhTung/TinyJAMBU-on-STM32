#ifndef PROTOCOL_CONFIG_H
#define PROTOCOL_CONFIG_H

#include <stdint.h>
#include "project_config.h"

/* Các hằng số giao thức Frame */
#define PROTOCOL_START_BYTE            FRAME_START_BYTE /* 0xA5U */
#define PROTOCOL_HEADER_SIZE           4U   /* START(1) + LEN(2) + TYPE(1) */
#define PROTOCOL_NONCE_SIZE            NONCE_SIZE_BYTES /* 12U */
#define PROTOCOL_TAG_SIZE              TAG_SIZE_BYTES   /* 8U */
#define PROTOCOL_CRC_SIZE              2U   /* CRC16 (2 bytes) */

/* Tổng số byte phụ trợ (Overhead): 4 + 12 + 8 + 2 = 26 bytes */
#define PROTOCOL_OVERHEAD_SIZE         (PROTOCOL_HEADER_SIZE + PROTOCOL_NONCE_SIZE + PROTOCOL_TAG_SIZE + PROTOCOL_CRC_SIZE)

/* Kích thước gói tin lớn nhất có thể truyền */
#define PROTOCOL_MAX_FRAME_SIZE        (PROTOCOL_OVERHEAD_SIZE + MAX_PAYLOAD_SIZE)

/* Định nghĩa các loại bản tin (Message Types) */
typedef enum {
    FRAME_TYPE_DATA        = 0x01, /* Dữ liệu telemetry cảm biến */
    FRAME_TYPE_ALERT       = 0x02, /* Cảnh báo khẩn cấp */
    FRAME_TYPE_ACK         = 0x03  /* Xác nhận từ ESP32 */
} frame_type_t;

#endif /* PROTOCOL_CONFIG_H */
