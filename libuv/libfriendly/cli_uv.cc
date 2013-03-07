// XXX temporary

#include <sys/socket.h>

#include "http_io.h"

int
main(int argc, const char* const* argv)
{
    http_io_signal_cb cb;
    int fd;
    int res;

    http_io_t* http = http_io_create();
    http_io_signal_get(http, &fd, &cb);
    res = send(fd, &fd, sizeof(fd), 0);
    http_io_destroy(&http);

    return 0;
}
