//

#include <stdio.h>
#include <stdlib.h>
#include <uzmtp/uzmtp.h>

int main(int argc, char *argv[]);
int print_usage();
int die_with_error(int, char *);

int main(int argc, char *argv[]) {
    if (argc != 3 || (!(argv[1] && !memcmp(argv[1], "tcp://", 6)))) {
	print_usage();
	return -1;
    }

    // Allocate dealer socket ctx.
    UzmtpDealer *d = uzmtp_dealer_new();
    if (!d) return die_with_error(-1, "memory");

    // Connect to rep socket.
    if (uzmtp_dealer_connect_endpoint(d, argv[1])) {
	uzmtp_dealer_free(&d);
	return die_with_error(-2, "Can't connect to server!");
    }

    // Allocate message
    UzmtpMsg *msg = uzmtp_msg_from_const_data(0, argv[2], strlen(argv[2]));
    if (!msg) {
	uzmtp_dealer_free(&d);
	return die_with_error(-3, "memory");
    }

    // Send message
    if (uzmtp_dealer_send(d, msg)) {
	uzmtp_dealer_free(&d);
	uzmtp_msg_destroy(&msg);
	return die_with_error(-4, "network error!");
    }

    // TODO - wait for response!

    // Clean
    uzmtp_dealer_free(&d);
    uzmtp_msg_destroy(&msg);
    return 0;
}
int print_usage() {
    printf("%s\n",
	   "USAGE: uzmtp-client tcp://x.x.x.x:y '{\"hello\":\"world\"}'");
    return 0;
}
int die_with_error(int code, char *err) {
    char errmsg[100];
    snprintf(errmsg, 100, "error[%d] [%s]", code, err);
    printf("%s\n", errmsg);
    return code;
}
