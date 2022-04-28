#include <stdint.h>
#include <zephyr.h>

#include <assert.h>
#include <logging/log.h>
#include <net/net_if.h>
#include <net/socket.h>
#include <net/socket_net_mgmt.h>

#include <uzmtp/uzmtp.h>

#define MAX_BUF_LEN 1024
#define STACK_SIZE 4096

#if defined(CONFIG_DEMO_REMOTE_IPV4)
#define SERVER CONFIG_DEMO_IPV4
#else
//#define SERVER "192.168.0.31"
#define SERVER "192.0.2.2"
#endif

#if defined(CONFIG_DEMO_REMOTE_PORT)
#define SERVER CONFIG_DEMO_PORT
#else
#define PORT 33558
#endif

LOG_MODULE_REGISTER(main);

static ssize_t
sendall(int sock, const void* buf, size_t len)
{
    while (len) {
        ssize_t out_len = send(sock, buf, len, 0);

        if (out_len <= 0) {
            if (errno == EAGAIN) continue;
            return out_len;
        }
        buf = (const char*)buf + out_len;
        len -= out_len;
    }

    return 0;
}

static int
demo_connect(int fd, const char* server, int port)
{
    struct sockaddr sockaddr;
    int ret = -1;

    memset(&sockaddr, 0, sizeof(sockaddr));
    net_sin(&sockaddr)->sin_family = AF_INET;
    net_sin(&sockaddr)->sin_port = htons(port);
    inet_pton(AF_INET, server, &net_sin(&sockaddr)->sin_addr);

    LOG_INF("Attempting to connect to %s:%d", server, port);
    ret = connect(fd, &sockaddr, sizeof(sockaddr));
    if (ret < 0) {
        LOG_ERR("Cannot connect to %s:%d", SERVER, PORT);
        LOG_ERR("Error code: %d", errno);
    }
    return ret;
}

static int
demo_recv(uzmtp_dealer_s* dealer, uint8_t* buf, size_t l)
{
    int ret, fd = *(int*)uzmtp_dealer_connection_get(dealer);

    LOG_INF("Receiving");
    ret = recv(fd, buf, sizeof(buf), 0);
    LOG_INF("Recv (%d)", ret);
    if (ret <= 0) {
        LOG_ERR("Recv error (%d) (%d)", ret, errno);
        return ret;
    }
    ret = uzmtp_dealer_parse(dealer, buf, ret);
    if (ret) {
        LOG_ERR("Uzmtp parse error (%d)", ret);
    }
    return ret;
}

static int
demo_cb_want_write(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    int connection = *(int*)uzmtp_dealer_connection_get(dealer);
    // return sendall(connection, b, sz) == sz ? 0 : -1;
    return sendall(connection, b, sz);
}

static int
demo_cb_on_recv(uzmtp_dealer_s* dealer, uint32_t n)
{
    (*(int*)uzmtp_dealer_context_get(dealer))++;
    while (n--) {
        uzmtp_msg_s* msg = uzmtp_dealer_pop_incoming(dealer);
        assert(msg);
        LOG_INF(
            "Received: [%.*s]", (int)uzmtp_msg_size(msg), uzmtp_msg_data(msg));
        uzmtp_msg_destroy(&msg);
    }

    return 0;
}

static void
demo_cb_on_error(uzmtp_dealer_s* dealer, EUZMTP_ERROR error)
{
    ((void)dealer);
    LOG_ERR("UZMTP ERROR %d", error);
    LOG_ERR("Error code: %d", errno);
}

uzmtp_dealer_settings demo_settings = { //
    .want_write = demo_cb_want_write,
    .on_recv = demo_cb_on_recv,
    .on_error = demo_cb_on_error
};

void
main(void)
{
    uzmtp_dealer_s* dealer;
    uzmtp_msg_s* msg;
    uint8_t buf[MAX_BUF_LEN];
    int fd, ret, state = 0;

    LOG_INF("Starting application...");

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        LOG_ERR("Fatel error, failed to create %s:%d socket", SERVER, PORT);
        exit(fd);
    }

    ret = demo_connect(fd, SERVER, PORT);
    if (ret < 0) {
        LOG_ERR("Fatal error, failed to connect to %s:%d", SERVER, PORT);
        close(fd);
        exit(ret);
    }

    LOG_INF("Connected to %s:%d", SERVER, PORT);

    dealer = uzmtp_dealer_new(&demo_settings);
    if (!dealer) {
        LOG_ERR("Fatal error, failed to create dealer socket");
        close(fd);
        exit(-1);
    }

    ret = uzmtp_dealer_connect(dealer, &fd);
    if (ret) {
        LOG_ERR("Fatal error, failed to connect to router socket");
        uzmtp_dealer_destroy(&dealer);
        close(fd);
        exit(-1);
    }

    LOG_INF("Dealer handshake started...");
    while (!uzmtp_dealer_ready(dealer)) demo_recv(dealer, buf, sizeof(buf));
    LOG_INF("Dealer handshake complete...");

    uzmtp_dealer_context_set(dealer, &state);

    while (1) {
        k_msleep(1000);
        if (state == 0) {
            // Write to peer
            msg = uzmtp_msg_new_from_const_data(UZMTP_MSG_MORE, "hello", 5);
            if (!msg) break;
            ret = uzmtp_dealer_send(dealer, &msg);
            LOG_INF("hello sent... (%d)", ret);
            if (msg) break;
            msg = uzmtp_msg_new_from_const_data(0, "world", 5);
            if (!msg) break;
            ret = uzmtp_dealer_send(dealer, &msg);
            LOG_INF("world sent... (%d)", ret);
            if (msg) break;

            state++;
        }

        // Receive from peer
        ret = demo_recv(dealer, buf, sizeof(buf));
        if (ret < 0) {
            LOG_ERR("Exiting from recv error (%d)", errno);
            break;
        }

        // end app
        if (state == 2) break;
    }

    LOG_INF("Application complete %s", state == 2 ? "success" : "fail");

    uzmtp_dealer_destroy(&dealer);
    close(fd);
    //exit(state == 2 ? 0 : state);
}
