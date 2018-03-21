/**
 * @file uzmtp_types.h
 *
 * @brief Type definitions shared by public and private routines for uzmtp
 * library.
 */
#ifndef UZMTP_TYPES_H_
#define UZMTP_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UZMTP_CONFIG
#define UZMTP_CONFIG "uzmtp_config_default.h"
#endif

#include UZMTP_CONFIG

typedef struct uzmtp_dealer__s uzmtp_dealer_s;
typedef struct uzmtp_msg__s uzmtp_msg_s;
typedef void uzmtp_connection;

typedef enum {
    UZMTP_MSG_MORE = 1,
    UZMTP_MSG_LARGE = 2,
    UZMTP_MSG_COMMAND = 4
} EUZMTP_MSG;

// State machine
typedef enum {
    UZMTP_NULL = 0,
    UZMTP_CONNECT_WANT_GREETING = 1,
    UZMTP_CONNECT_WANT_READY,
    UZMTP_RECV_FLAGS,
    UZMTP_RECV_LENGTH,
    UZMTP_RECV_BODY
} EUZMTP_STATE;

typedef enum {
    UZMTP_ERROR_VERSION = 0,
    UZMTP_ERROR_MEMORY,
    UZMTP_ERROR_SEND,
    UZMTP_ERROR_RECV,
    UZMTP_ERROR_PROTOCOL
} EUZMTP_ERROR;

typedef int (*uzmtp_want_write_fn)(uzmtp_dealer_s*, const uint8_t*, uint32_t);
typedef int (*uzmtp_recv_fn)(uzmtp_dealer_s*, uint32_t);
typedef void (*uzmtp_error_fn)(uzmtp_dealer_s*, EUZMTP_ERROR);

// Context callbacks
typedef struct
{
    uzmtp_want_write_fn want_write;
    uzmtp_recv_fn on_recv;
    uzmtp_error_fn on_error;
} uzmtp_dealer_settings;

#ifdef __cplusplus
}
#endif
#endif
