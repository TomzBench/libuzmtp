/*
 * uzmtp_settings.h
 */

#ifndef UZMTP_SETTINGS_H_
#define UZMTP_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Uncomment next line if using LINUX */
/* #define UZMTP_USE_LINUX */

/* Uncomment next line if using MQX */
/* #define UZMTP_USE_MQX */

#ifdef UZMTP_USE_LINUX
#define uzmtp_malloc malloc
#define uzmtp_free free
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#endif

#ifdef UZMTP_USE_MQX
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <ipcfg.h>
#define MSG_DONTWAIT RTCS_MSG_BLOCK
#define uzmtp_malloc _mem_alloc
#define uzmtp_free _mem_free
#define assert(x) 
#endif

#ifdef __cplusplus
}
#endif

#endif
