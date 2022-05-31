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

// 2 data pointers (*next, *data)
// 2 bytes         (pad[1], flags)
// 1 int           (greedy)
// size_t          (size)
#define UZMTP_MSG_SIZE                                                         \
    (sizeof(uintptr_t) * 2) + (sizeof(uint8_t) * 2) + (sizeof(int)) +          \
        (sizeof(size_t))

// #define UZMTP_MSG_SIZE                                                         \
//     (sizeof(uintptr_t)) + (sizeof(uint8_t)) + sizeof(uint8_t) +                \
//         sizeof(size_t) + sizeof(int)

typedef struct uzmtp_dealer__s uzmtp_dealer_s;
typedef struct uzmtp_msg__s uzmtp_msg_s;

// You might want to initialize msgs on the stack. since uzmtp_msg__s is an
// opaque type, use this with the init functions instead of new functions
#define uzmtp_msg_opaque(_n)                                                   \
    struct                                                                     \
    {                                                                          \
        union                                                                  \
        {                                                                      \
            max_align_t a;                                                     \
            char __bytes[UZMTP_MSG_SIZE + _n];                                 \
        };                                                                     \
    }

// Helper to point to the end of the message (which contains the data)
#define uzmtp_msg_pad(_msg) &((uint8_t*)_msg)[UZMTP_MSG_SIZE]

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
