#include <assert.h>
#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

#include <uzmtp/uzmtp.h>

static int
demo_cb_want_write(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
}

static int
demo_callback_on_recv(uzmtp_dealer_s* dealer, uint32_t n)
{
    while (n--) {
        uzmtp_msg_s* msg = uzmtp_dealer_pop_incoming(dealer);
        assert(msg);
        printf("%.*s", uzmtp_msg_size(msg), uzmtp_msg_data(msg));
    }
}

void
main(void)
{
    LOG_INF("Starting application...");
    while (1) {
        k_msleep(1000);
    }
}
