/*
 * main.c
 */

#include "uzmtp.h"

#ifdef UZMTP_USE_LINUX

int s_interrupted = 0;
static void s_signal_handler(int signal_value);
static void s_catch_signals(void);

static void s_signal_handler(int x) { s_interrupted = 1; }
static void s_catch_signals(void) {
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

int main(int argc, char **argv);

int main(int argc, char **argv) {
    ((void)argc);
    ((void)argv);

    s_catch_signals();

    // Setup dealer
    UzmtpDealer *dealer = uzmtp_dealer_new();
    int ret = uzmtp_dealer_connect(dealer, "127.0.0.1", 5555);
    if (ret != 0) {
	printf("connection failed!\n");
    }

    // Poll connection
    while (true) {
	ret = uzmtp_dealer_poll(dealer, 10);
	if (s_interrupted) {
	    break;
	}
    }

    // Free dealer
    uzmtp_dealer_free(&dealer);
    return -1;
}

#endif

//
//
//
