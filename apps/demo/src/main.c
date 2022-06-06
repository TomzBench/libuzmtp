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
demo_recv_ready(uzmtp_dealer_s* dealer)
{
    uint8_t buf[64];
    uzmtp_msg_s msg;
    int ret, fd = *(int*)uzmtp_dealer_connection(dealer);
    int c = 0;

    while (!uzmtp_dealer_ready(dealer)) {
        LOG_INF("Receiving");
        ret = recv(fd, &buf[c], sizeof(buf) - c, 0);
        LOG_INF("Recv (%d)", ret);
        if (ret <= 0) {
            LOG_ERR("Recv error (%d) (%d)", ret, errno);
            return ret;
        }
        ret = uzmtp_dealer_parse(dealer, &buf[c], ret, &msg, 1);
        if (!(ret == UZMTP_WANT_MORE)) break;
    }
    return ret;
}

static int
demo_cb_want_write(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    int connection = *(int*)uzmtp_dealer_connection(dealer);
    // return sendall(connection, b, sz) == sz ? 0 : -1;
    return sendall(connection, b, sz);
}

void
main(void)
{
    uzmtp_dealer_s dealer;
    uint8_t buf[MAX_BUF_LEN];
    uzmtp_msg_s in[2];
    uzmtp_msg_s out;
    int fd, n = 0, ret, recvd, state = 0;

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

    uzmtp_dealer_init(&dealer, demo_cb_want_write, NULL);

    ret = uzmtp_dealer_connect(&dealer, &fd);
    if (ret) {
        LOG_ERR("Fatal error, failed to connect to router socket");
        uzmtp_dealer_deinit(&dealer);
        close(fd);
        exit(-1);
    }

    LOG_INF("Dealer handshake started...");
    demo_recv_ready(&dealer);
    LOG_INF("Dealer handshake complete...");

    while (1) {
        k_msleep(200);
        if (state == 0) {
            // Write to peer
            uzmtp_msg_init_str(&out, UZMTP_MSG_MORE, "hello");
            ret = uzmtp_dealer_send(&dealer, &out);
            LOG_INF("hello sent... (%d)", ret);
            uzmtp_msg_init_str(&out, 0, "world");
            ret = uzmtp_dealer_send(&dealer, &out);
            LOG_INF("world sent... (%d)", ret);

            state++;
        }

        // Receive from peer
        recvd = recv(fd, &buf[n], sizeof(buf) - n, 0);
        if (recvd <= 0) {
            LOG_ERR("Recv error (%d) (%d)", recvd, errno);
            ret = -1;
            break;
        }
        ret = uzmtp_dealer_parse(&dealer, &buf[n], recvd, in, 2);
        if (ret == UZMTP_WANT_MORE) {
            n += recvd;
        }
        else if (ret >= 0) {
            break;
        }
    }

    if (uzmtp_msg_size(&in[0]) == 5 && uzmtp_msg_size(&in[1]) == 5 &&
        !memcmp(uzmtp_msg_data(&in[0]), "HELLO", 5) &&
        !memcmp(uzmtp_msg_data(&in[1]), "WORLD", 5)

    ) {
        LOG_INF("Application complete %s", "SUCCESS");
    }
    else {
        LOG_ERR("Application complete %s", "FAIL");
    }

    uzmtp_dealer_deinit(&dealer);
    close(fd);
}
