#include "czmq.h"
#include <ctype.h>

int do_echo(zsock_t* sock, zmsg_t** msg_p);

void
echo_task(zsock_t* pipe, void* args)
{
    ((void)args);
    int err;
    bool terminated;
    char* command;
    zsock_t* sock;
    zmsg_t* msg;
    zmq_pollitem_t items[2];

    sock = zsock_new_router("tcp://127.0.0.1:8888");
    if (!sock) return;

    // Signal parent
    zsock_signal(pipe, 0);

    terminated = false;
    while (!terminated) {

        // Set up poll for listeners, handle terminate or echo
        items[0].fd = items[0].revents = 0;
        items[0].events = ZMQ_POLLIN;
        items[0].socket = zsock_resolve(pipe);
        items[1].fd = items[1].revents = 0;
        items[1].events = ZMQ_POLLIN;
        items[1].socket = zsock_resolve(sock);

        err = zmq_poll(items, 2, 100);
        if (err < 0) break;

        if (items[0].revents && ZMQ_POLLIN) {
            msg = zmsg_recv(pipe);
            if (!msg) break;
            command = zmsg_popstr(msg);
            if (strlen(command) == 5 && (!(memcmp(command, "$TERM", 5)))) {
                terminated = 1;
            }
            freen(command);
            zmsg_destroy(&msg);
        }
        if (items[1].revents && ZMQ_POLLIN) {
            msg = zmsg_recv(sock);
            if (!msg) break;
            err = do_echo(sock, &msg);
            if (err) break;
        }
    }
    zsock_destroy(&sock);
}

int
do_echo(zsock_t* sock, zmsg_t** msg_p)
{
    int err, count = 0, sz;
    zframe_t* frame;
    zmsg_t *outgoing = NULL, *msg = *msg_p;

    outgoing = zmsg_new();
    if (!outgoing) return -1;

    count = zmsg_size(msg);

    // Build echo
    for (int i = 0; i < count; i++) {
        frame = zmsg_pop(msg);
        if (frame) {
            if (i == 0) {
                err = zmsg_append(outgoing, &frame);
            } else {
                sz = zframe_size(frame);
                for (int c = 0; c < sz; c++) {
                    zframe_data(frame)[c] = toupper(zframe_data(frame)[c]);
                }
                err = zmsg_append(outgoing, &frame);
            }
            if (err) {
                zmsg_destroy(&msg);
                zmsg_destroy(&outgoing);
                zframe_destroy(&frame);
                break;
            }
        } else {
            zmsg_destroy(&msg);
            zmsg_destroy(&outgoing);
            break;
        }
    }
    zmsg_destroy(msg_p);
    if (outgoing) {
        err = zmsg_send(&outgoing, sock);
        if (err) zmsg_destroy(&outgoing);
        return err;
    } else {
        return -1;
    }
}
