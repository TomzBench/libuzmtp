/*
 * uzmtp_msg.h
 */

#ifndef UZMTP_MSG_H_
#define UZMTP_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uzmtp_settings.h"

#define UZMTP_ANYSIZE_ARRAY 1
typedef struct _UzmtpMsg {
    uint8_t flags;
    uint8_t *data;
    size_t size;
    int greedy;
    uint8_t pad[UZMTP_ANYSIZE_ARRAY];
} _UzmtpMsg;

typedef enum {
    UZMTP_MSG_MORE = 1,
    UZMTP_MSG_LARGE = 2,
    UZMTP_MSG_COMMAND = 4
} EUZMTP_MSG;

_UzmtpMsg *uzmtp_msg_new(uint8_t flags, size_t size);

_UzmtpMsg *uzmtp_msg_from_data(uint8_t flags, uint8_t **data_p, size_t size);

_UzmtpMsg *uzmtp_msg_from_const_data(uint8_t flags, void *data, size_t size);

void uzmtp_msg_destroy(_UzmtpMsg **self_p);

uint8_t uzmtp_msg_flags(_UzmtpMsg *self);
void uzmtp_msg_set_more(_UzmtpMsg *self);
void uzmtp_msg_clr_more(_UzmtpMsg *self);
bool uzmtp_msg_more(_UzmtpMsg *self);
uint8_t *uzmtp_msg_data(_UzmtpMsg *self);
size_t uzmtp_msg_size(_UzmtpMsg *self);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
