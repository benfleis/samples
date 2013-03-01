#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ev.h"

static ev_io stdin_watcher;
static ev_io stdout_watcher;
static char stdout_buf[4096];
static size_t stdout_buf_pos = 0;

static void
log(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    stdout_buf_pos += vsnprintf(stdout_buf + stdout_buf_pos,
        sizeof(stdout_buf) - stdout_buf_pos, fmt, ap);
    stdout_buf[stdout_buf_pos] = 0;     // just for debugging
    ev_io_start(EV_DEFAULT_UC_ &stdout_watcher);
    va_end(ap);
}

static void
stdin_cb(EV_P_ ev_io* watcher, int revents)
{
    char buf[4096];
    int rv;

    rv = read(STDIN_FILENO, buf, sizeof(buf));
    if (rv >= 0) {
        log("read()'d %d bytes\n", rv);
        if (rv == 0) {
            log("Exiting.\n");
            ev_io_stop(EV_DEFAULT_UC_ &stdin_watcher);
            // Don't break here -- let log work its way out stderr, and the
            // event loop naturally terminate when there are no remaining
            // events.
            //ev_break(EV_A_ EVBREAK_ALL);
        }
    }
    else if (errno != EAGAIN && errno != EINPROGRESS) {
        fprintf(stderr, "read() returned error: %s", strerror(errno));
        exit(1);
    }
}

static void
stdout_cb(EV_P_ ev_io* watcher, int wevents)
{
    int rv;

    rv = write(STDOUT_FILENO, stdout_buf, stdout_buf_pos);
    if (rv == stdout_buf_pos)
        ev_io_stop(EV_DEFAULT_UC_ &stdout_watcher);
    else if (rv >= 0)
        memmove(stdout_buf, stdout_buf + stdout_buf_pos, stdout_buf_pos);
    else {
        fprintf(stderr, "write() returned error: %s", strerror(errno));
        exit(1);
    }
    stdout_buf_pos = 0;
}

static int
set_non_blocking(int fd)
{
  int val = fcntl(fd, F_GETFL, 0);
  return val == -1 ? -1 : fcntl(fd, F_SETFL, val | O_NONBLOCK);
}

int
main(int argc, const char* const* argv)
{
    printf("GIMME YOUR INPUTS!\n");

    // set stdin/out to nonblocking
    set_non_blocking(STDIN_FILENO);
    set_non_blocking(STDOUT_FILENO);
    ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
    ev_io_init(&stdout_watcher, stdout_cb, STDOUT_FILENO, EV_WRITE);

    struct ev_loop* loop = EV_DEFAULT;
    ev_io_start(loop, &stdin_watcher);
    ev_run(loop, 0);
    return 0;
}
