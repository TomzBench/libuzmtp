#include "zmq.hpp"
#include <signal.h>
#include <iostream>
#include <string>

int s_interrupted = 0;
static void s_signal_handler(int signal_value);
static void s_catch_signals(void);
int main();
static void s_signal_handler(int) { s_interrupted = 1; }

static void s_catch_signals(void) {
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

int main() {
    s_catch_signals();
    zmq::context_t ctx(1);
    zmq::socket_t sock(ctx, ZMQ_ROUTER);
    sock.bind("tcp://*:5555");

    std::cout << "listening on port 5555" << std::endl;

    while (true) {
	zmq::message_t msg;
	try {
	    sock.recv(&msg);
	} catch (zmq::error_t& e) {
	    std::cout << "Interrupted!" << std::endl;
	}
	if (s_interrupted) {
	    break;
	}
    }

    std::cout << "Exiting()" << std::endl;
}
