// XXX temporary

#include <cassert>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>

#include "http_io.h"


static void
send_cb(int status, void* response, uint32_t len)
{
    fprintf(stderr, "send_cb(%d, %ull, ...)\n", status, len);
}

int
main(int argc, const char* const* argv)
{
    http_io_signal_cb cb;
    int fd;
    int res;
    char msg[] = "/?q=ASFD";
    const int len = strlen(msg);

    //http_io_t* http = http_io_create("74.125.132.94", 80);  // http://www.google.nl
    http_io_t* http = http_io_create("127.0.0.1", 6600);
    http_io_signal_get(http, &fd, &cb);

    res = http_io_send(http, (void*)msg, len, send_cb);
    assert(res == 0);
    /*
    msg[5]++;
    res = http_io_send(http, (void*)msg, len, send_cb);
    assert(res == 0);
    msg[5]++;
    res = http_io_send(http, (void*)msg, len, send_cb);
    assert(res == 0);
    msg[5]++;
    res = http_io_send(http, (void*)msg, len, send_cb);
    assert(res == 0);
    */

    http_io_destroy(&http);

    return 0;
}
