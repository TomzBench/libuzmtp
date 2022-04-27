#include <uzmtp/uzmtp.h>
#include <zmq.h>

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

// break loop when integration test is over
static int test_passed = 0, test_completed = 0;

// demo callbacks
int
demo_callback_want_write(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
int connection = *(int*)uzmtp_dealer_connection_get(dealer);
return send(connection, b, sz, 0) == sz ? 0 : -1;
}

int
demo_callback_on_recv(uzmtp_dealer_s* dealer, uint32_t n)
{
    int pass = 0;
    uzmtp_msg_s* msg;

    // fail
    if (!(n == 2)) return -1;

    // Verify hello echo
    msg = uzmtp_dealer_pop_incoming(dealer);
    if (msg) {
        if (((uzmtp_msg_size(msg) == 5)) &&
            (!memcmp(uzmtp_msg_data(msg), "HELLO", 5))) {
            pass++;
        }
        uzmtp_msg_destroy(&msg);
    }

    // Verify world echo
    msg = uzmtp_dealer_pop_incoming(dealer);
    if (msg) {
        if (((uzmtp_msg_size(msg) == 5)) &&
            (!memcmp(uzmtp_msg_data(msg), "WORLD", 5))) {
            pass++;
        }
        uzmtp_msg_destroy(&msg);
    }

    test_completed = 1;
    test_passed = pass == 2 ? 1 : 0;

    return 0;
}

void
demo_callback_on_error(uzmtp_dealer_s* dealer, EUZMTP_ERROR error)
{
    ((void)dealer);
    ((void)error);
}

uzmtp_dealer_settings demo_settings = { //
    .want_write = demo_callback_want_write,
    .on_recv = demo_callback_on_recv,
    .on_error = demo_callback_on_error
};

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    uint8_t buffer[1024];
    struct sockaddr_in addr;
    int connection, err, sz, echo_sent = 0;
    uzmtp_dealer_s* dealer;
    uzmtp_msg_s* msg;
    const char* ip = "127.0.0.1";
    uint32_t port = 33558;

    // Create a socket
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
    connection = socket(AF_INET, SOCK_STREAM, 0);
    if (connection < 0) {
        close(connection);
        return -1;
    }

    // connect to zmq echo server
    if (connect(connection, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(connection);
        return -1;
    }

    // Create dealer instance
    dealer = uzmtp_dealer_new(&demo_settings);
    if (!dealer) {
        close(connection);
        return -1;
    }

    // Connect our dealer to socket
    err = uzmtp_dealer_connect(dealer, &connection);
    if (err) {
        uzmtp_dealer_destroy(&dealer);
        close(connection);
        return -1;
    }

    while (!test_completed) {
        if (uzmtp_dealer_ready(dealer) && !echo_sent) {
            // Send frames echo server, expect HELLO WORLD in our recv callback
            echo_sent = 1;

            msg = uzmtp_msg_new_from_const_data(UZMTP_MSG_MORE, "hello", 5);
            if (!msg) break;
            uzmtp_dealer_send(dealer, &msg);
            if (msg) break;
            msg = uzmtp_msg_new_from_const_data(0, "world", 5);
            if (!msg) break;
            uzmtp_dealer_send(dealer, &msg);
            if (msg) break;
        }

        sz = recv(connection, buffer, sizeof(buffer), 0);
        if (sz <= 0) break;
        err = uzmtp_dealer_parse(dealer, buffer, sz);
        if (err) break;
    }

    uzmtp_dealer_destroy(&dealer);
    close(connection);
    return test_passed ? 0 : -1;
}
