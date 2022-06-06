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
demo_send(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    int connection = *(int*)uzmtp_dealer_connection(dealer);
    return send(connection, b, sz, 0) == sz ? 0 : -1;
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    uint8_t buff[1024];
    struct sockaddr_in addr;
    int connection, err, n = 0, sz, echo_sent = 0;
    uzmtp_dealer_s dealer;
    uzmtp_msg_s out, in[2];
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
    uzmtp_dealer_init(&dealer, demo_send, NULL);

    // Connect our dealer to socket
    err = uzmtp_dealer_connect(&dealer, &connection);
    if (err) {
        uzmtp_dealer_deinit(&dealer);
        close(connection);
        return -1;
    }

    while (!test_completed) {
        if (uzmtp_dealer_ready(&dealer) && !echo_sent) {
            echo_sent = 1;
            // Send frames echo server, expect HELLO WORLD in our recv callback

            uzmtp_msg_init_str(&out, UZMTP_MSG_MORE, "hello");
            uzmtp_dealer_send(&dealer, &out);
            uzmtp_msg_init_str(&out, 0, "world");
            uzmtp_dealer_send(&dealer, &out);
        }

        sz = recv(connection, &buff[n], sizeof(buff) - n, 0);
        if (sz <= 0) break;
        err = uzmtp_dealer_parse(&dealer, &buff[n], sz, in, 2);
        if (err == UZMTP_WANT_MORE) {
            n += err;
        }
        else if (err >= 0) {
            break;
        }
    }

    if ((uzmtp_msg_size(&in[0]) == 5) && (uzmtp_msg_size(&in[1]) == 5) &&
        (!memcmp(uzmtp_msg_data(&in[0]), "HELLO", 5)) &&
        (!memcmp(uzmtp_msg_data(&in[1]), "WORLD", 5))) {
        test_passed = 1;
    }

    uzmtp_dealer_deinit(&dealer);
    close(connection);
    return test_passed ? 0 : 1;
}
