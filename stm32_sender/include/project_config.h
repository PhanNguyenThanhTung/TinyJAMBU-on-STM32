#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H
/* System clock */
#define HSE_CLOCK_HZ               8000000UL
#define SYSTEM_CLOCK_HZ           72000000UL
/* UART */
#define UART_BAUD_RATE             115200UL
#define UART_TX_BUFFER_SIZE        256U
#define UART_RX_BUFFER_SIZE        256U
/* UART tranfer config */
#define FRAME_START_BYTE           0xA5U
#define MAX_PAYLOAD_SIZE           128U /* TiniJAMBE 128-bit */
/* TinyJAMBU config  */
#define KEY_SIZE_BYTES             16U
#define NONCE_SIZE_BYTES           12U
#define TAG_SIZE_BYTES             8U
/* Watchdog config */
#define WATCHDOG_TIMEOUT_MS        2000U /* Timeout 2s */
/* Data config */
#define DATA_SEND_PERIOD_MS        1000U

#endif 