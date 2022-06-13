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

#include "uzmtp_platform.h"

#define UZMTP_MSG_SIZE sizeof(uintptr_t) + sizeof(size_t) + sizeof(uint8_t)
#define UZMTP_DEALER_SIZE                                                      \
    sizeof(uintptr_t) * 3 + sizeof(int) + sizeof(size_t) + 13

typedef struct uzmtp_msg_s
{
    union
    {
        uzmtp_max_align a;
        char __bytes[UZMTP_MSG_SIZE];
    };
} uzmtp_msg_s;

typedef struct uzmtp_dealer_s
{
    union
    {
        uzmtp_max_align a;
        char __bytes[UZMTP_DEALER_SIZE];
    };
} uzmtp_dealer_s;

typedef void uzmtp_connection;

typedef enum
{
    UZMTP_MSG_MORE = 1,
    UZMTP_MSG_LARGE = 2,
    UZMTP_MSG_COMMAND = 4
} EUZMTP_MSG;

// State machine
typedef enum
{
    UZMTP_NULL = 0,
    UZMTP_CONNECT_WANT_GREETING = 1,
    UZMTP_CONNECT_WANT_READY,
    UZMTP_RECV_FLAGS,
    UZMTP_RECV_LENGTH,
    UZMTP_RECV_BODY
} EUZMTP_STATE;

typedef enum
{
    UZMTP_ERROR_VERSION = -2,
    UZMTP_ERROR_OVERFLOW = -3,
    UZMTP_ERROR_SEND = -4,
    UZMTP_ERROR_RECV = -5,
    UZMTP_ERROR_PROTOCOL = -6,
    UZMTP_WANT_MORE = -32
} EUZMTP_ERROR;

typedef int (*uzmtp_send_fn)(uzmtp_dealer_s*, const uint8_t*, uint32_t);

#ifdef __cplusplus
}
#endif
#endif
