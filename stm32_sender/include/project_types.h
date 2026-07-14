#ifndef PROJECT_TYPES_H
#define PROJECT_TYPES_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/* Define status of project*/
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR,
    STATUS_TIMEOUT,
    STATUS_BUSY,
    STATUS_INVALID_PARAMETER,
    STATUS_BUFFER_OVERFLOW,
    STATUS_AUTHENTICATION_FAILED
} status_t;

typedef struct {
    uint8_t * data;
    uint32_t length;
} buffer_t;

typedef void (*callback_t)(void);

#endif
