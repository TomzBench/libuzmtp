/*
 * uzmtp_settings.h
 */

#ifndef UZMTP_H_
#define UZMTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uzmtp_settings.h"

typedef struct _UzmtpDealer UzmtpDealer;
typedef struct _UzmtpMsg UzmtpMsg;
typedef struct _UzmtpSocket UzmtpSocket;

/**
 * @brief Create a new instance of a dealer socket. Return object required for
 * rest of API usage. Caller must free dealer with uzmtp_dealer_free().
 *
 * @return Heap pointer if successful. NULL if error.
 */
extern UzmtpDealer *uzmtp_dealer_new();

/**
 * @brief Free instance of dealer class object from heap.  Once dealer is freed
 * from heap, the callers reference to heap is set to NULL.
 *
 * @param dealer_p Address of pointer to dealer socket.
 */
extern void uzmtp_dealer_free(UzmtpDealer **);

/**
 * @brief Connect to a remote zmtp listener.
 *
 * @param dealer Pointer to heap object of dealer socket created with
 * uzmtp_dealer_new()
 * @param endpoint endpoint string in the format "tcp://127.0.0.1:5555"
 *
 * @return 0 if successful.
 */
extern int uzmtp_dealer_connect_endpoint(UzmtpDealer *, const char *);

/**
 * @brief Connects to a remote zmtp listener.
 *
 * @param dealer Pointer to heap object of dealer socket created with
 * uzmtp_dealer_new().
 * @param host ip string in format "127.0.0.1".
 * @param port port number, IE 5555
 *
 * @return 0 if successful.
 */
extern int uzmtp_dealer_connect(UzmtpDealer *self, const char *host, int port);

/**
 * @brief Send message over network.
 *
 * @param dealer Pointer to heap object of dealer socket created with
 * uzmtp_dealer_new().
 * @param msg A message created from uzmtp_msg.c api.
 * (See:
 * 	uzmtp_msg_new()
 * 	uzmtp_msg_from_data()
 * 	uzmtp_msg_from_const_data()
 * )
 *
 * @return 0 if successful.
 */
extern int uzmtp_dealer_send(UzmtpDealer *, UzmtpMsg *);

extern UzmtpMsg *uzmtp_dealer_recv(UzmtpDealer *);


/**
 * @brief Poll socket for activity.
 *
 * @param dealer Pointer to heap object of dealer socket created with
 * uzmtp_dealer_new().
 * @param time How long to block and wait for socket activity.
 *
 * @return file descriptor if there is socket activity.  0 if no socket
 * activity.
 */
extern int uzmtp_dealer_poll(UzmtpDealer *self, int time);

extern int uzmtp_dealer_socket(UzmtpDealer *);

enum EUZMTP_MSG;
extern UzmtpMsg *uzmtp_msg_new(uint8_t flags, size_t size);
extern UzmtpMsg *uzmtp_msg_from_data(uint8_t flags, uint8_t **data_p,
				     size_t size);
extern UzmtpMsg *uzmtp_msg_from_const_data(uint8_t flags, void *data,
					   size_t size);
extern void uzmtp_msg_destroy(UzmtpMsg **self_p);
extern int uzmtp_msg_send(UzmtpMsg *msg, UzmtpSocket *sock);
extern UzmtpMsg *uzmtp_msg_recv(UzmtpSocket *);
extern uint8_t uzmtp_msg_flags(UzmtpMsg *self);
extern void uzmtp_msg_set_more(UzmtpMsg *self);
extern void uzmtp_msg_clr_more(UzmtpMsg *self);
extern bool uzmtp_msg_more(UzmtpMsg *self);
extern uint8_t *uzmtp_msg_data(UzmtpMsg *self);
extern size_t uzmtp_msg_size(UzmtpMsg *self);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UZMTP_H_ */
