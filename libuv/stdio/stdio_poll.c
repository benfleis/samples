#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "uv.h"

static uv_poll_t stdin_watcher;
static uv_poll_t stdout_watcher;

// buffer all log calls in this buf, so we can event output too!
static char log_buf[4096];
static size_t log_buf_pos = 0;

// this is only used for debugging, gdb p/s, etc.
#define log_buf_nul_term() \
    do { if (log_buf_pos < sizeof(log_buf)) log_buf[log_buf_pos] = 0; } while(0)

static void
stdout_cb(uv_poll_t* watcher, int status, int wevents)
{
    int rv;

    rv = write(STDOUT_FILENO, log_buf, log_buf_pos);
    if (rv == log_buf_pos) {
        // whole buf written, reset buf, clear writeable check
        uv_poll_stop(&stdout_watcher);
        log_buf_pos = 0;
    }
    else if (rv >= 0) {
        // partial buf written, shift and adjust pos (writeable check intact)
        memmove(log_buf, log_buf + rv, log_buf_pos - rv);
        log_buf_pos -= rv;
    }
    else {
        fprintf(stderr, "write() returned error: %s", strerror(errno));
        exit(1);
    }
    log_buf_nul_term();
}

static void
_log(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_buf_pos += vsnprintf(log_buf + log_buf_pos,
        sizeof(log_buf) - log_buf_pos, fmt, ap);
    va_end(ap);

    log_buf_nul_term();
    uv_poll_start(&stdout_watcher, UV_WRITABLE, stdout_cb); // turn on writeable check
}

static void
stdin_cb(uv_poll_t* watcher, int status, int revents)
{
    char buf[4096];
    int rv;

    rv = read(STDIN_FILENO, buf, sizeof(buf));
    if (rv >= 0) {
        _log("read()'d %d bytes\n", rv);
        if (rv == 0) {
            _log("Exiting on end of stream.\n");
            uv_poll_stop(&stdin_watcher);
            // Don't break here -- let _log work its way out stderr, and the
            // event loop naturally terminate when there are no remaining
            // events.
            //uv_break(uv_A_ EVBREAK_ALL);
        }
    }
    else if (errno != EAGAIN && errno != EINPROGRESS) {
        fprintf(stderr, "read() returned error: %s", strerror(errno));
        exit(1);
    }
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

    uv_loop_t* loop = uv_default_loop();
    uv_poll_init(loop, &stdin_watcher, STDIN_FILENO);
    uv_poll_init(loop, &stdout_watcher, STDOUT_FILENO);
    uv_poll_start(&stdin_watcher, UV_READABLE, stdin_cb);
    uv_run(loop, 0);
    return 0;
}
