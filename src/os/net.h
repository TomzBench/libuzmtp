#ifndef OS_NET_H_
#define OS_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../uzmtp_settings.h"

#ifdef UZMTP_USE_LINUX
#include "linux/net.h"
#endif

#ifdef UZMTP_USE_MQX
#include "mqx/net.h"
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
