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
#define SERVER "192.0.2.2"
#endif

#if defined(CONFIG_DEMO_REMOTE_PORT)
#define SERVER CONFIG_DEMO_PORT
#else
#define PORT 33558
#endif

LOG_MODULE_REGISTER(main);

static int
demo_cb_want_write(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    int connection = *(int*)uzmtp_dealer_connection_get(dealer);
    return send(connection, b, sz, 0) == sz ? 0 : -1;
}

static int
demo_cb_on_recv(uzmtp_dealer_s* dealer, uint32_t n)
{
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
    struct sockaddr sockaddr;
    uint8_t buf[MAX_BUF_LEN];
    uint32_t sz;
    int fd, ret;

    LOG_INF("Starting application...");

    memset(&sockaddr, 0, sizeof(sockaddr));

    net_sin(&sockaddr)->sin_family = AF_INET;
    net_sin(&sockaddr)->sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER, &net_sin(&sockaddr)->sin_addr);

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        LOG_ERR("Failed to create %s:%d socket", SERVER, PORT);
        exit(fd);
    }

    ret = connect(fd, &sockaddr, sizeof(sockaddr));
    if (ret < 0) {
        LOG_ERR("Cannot connect to %s:%d", SERVER, PORT);
        close(fd);
        exit(ret);
    }

    dealer = uzmtp_dealer_new(&demo_settings);
    if (!dealer) {
        LOG_ERR("Failed to create dealer socket");
        close(fd);
        exit(-1);
    }

    ret = uzmtp_dealer_connect(dealer, &fd);
    if (ret) {
        LOG_ERR("Failed to connect to router socket");
        uzmtp_dealer_destroy(&dealer);
        close(fd);
        exit(-1);
    }

    while (1) {
        k_msleep(1000);

        // Write to peer
        msg = uzmtp_msg_new_from_const_data(0, "hello", 5);
        if (!msg) break;
        uzmtp_dealer_send(dealer, &msg);
        if (msg) break;

        // Receive from peer
        sz = recv(fd, buf, sizeof(buf), 0);
        if (sz <= 0) break;
        ret = uzmtp_dealer_parse(dealer, buf, sz);
        if (ret) break;
    }

    uzmtp_dealer_destroy(&dealer);
    close(fd);
}
