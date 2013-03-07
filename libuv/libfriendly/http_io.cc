#include <cassert>
#include <cerrno>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>

#include "uv.h"

#include "http_io.h"


// ----------------------------------------------------------------------------

#define container_of(ptr, type, member) ({\
            const typeof( ((type *)0)->member ) *__mptr = (ptr);\
            (type *)( (char *)__mptr - offset_of(type,member) );})

#define offset_of(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// ----------------------------------------------------------------------------

struct request {
    // request bits
    size_t len;
    uint8_t* msg;

    // response bits
    struct {
        http_io_response_cb cb;
        int status;
        size_t len;
        uint8_t* msg;
    } response;
};

struct http_io {
    int signal_pair[2];     // socketpair fds; [0]=main_thread, [1]=io_thread
    pthread_t io_thread;
    pthread_mutex_t io_mutex;
    uv_loop_t* loop;

    uv_poll_t signal_read_watcher;
    uv_poll_t signal_write_watcher;

    http_io_response_cb response_cb;

    std::map<uint32_t, request> requests;
};

// ----------------------------------------------------------------------------

static int _signal_cb(http_io* ctx, uint64_t cookie)
{
    //uint32_t cmd = cookie >> 32;
    uint32_t id = cookie & 0xffffffff;

    // fetch context from hash table
    // call user callback on returned data, if specified.
    typedef std::map<uint32_t, request> request_by_id_t;
    request_by_id_t::iterator ri = ctx->requests.find(id);
    assert(ri != ctx->requests.end());

    return 0;
}

static void*
_run_io_thread(void* args)
{
    //fprintf(stderr, "_run_io_thread(...) {\n");
    http_io* ctx = static_cast<http_io*>(args);
    uv_run(ctx->loop, UV_RUN_DEFAULT);
    //fprintf(stderr, "} // _run_io_thread\n");
    return NULL;
}


static int
set_non_blocking(int fd)
{
  int val = fcntl(fd, F_GETFL, 0);
  return val == -1 ? -1 : fcntl(fd, F_SETFL, val | O_NONBLOCK);
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

static void
signal_read_cb(uv_poll_t* watcher, int status, int revents)
{
    fprintf(stderr, "signal_read_cb(...) { }\n");
    uint8_t buf[4096];
    ssize_t len = read(watcher->io_watcher.fd, buf, sizeof(buf));

    if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        // error
        fprintf(stderr, "read(fd, buf) -> %s", strerror(errno));
        uv_poll_stop(watcher);
    }
    if (len == 0) {
        // initiate shutdown
        uv_poll_stop(watcher);
    }
    else {
        // ignore buffering, just assume all is good.
        http_io* ctx = container_of(watcher, http_io, signal_read_watcher);
        if (ctx->response_cb)
            ctx->response_cb(200, len, buf);
    }

}

//static
void
signal_write_cb(uv_poll_t* watcher, int status, int wevents)
{
    fprintf(stderr, "signal_write_cb(...) { }\n");
}


// ----------------------------------------------------------------------------

http_io*
http_io_create(void)
{
    http_io* rv = new http_io();
    assert(rv);
    rv->loop = uv_loop_new();
    assert(rv->loop);

    // setup socket pair, then setup read/write checks for io_thread side
    int res = socketpair(PF_LOCAL, SOCK_DGRAM, 0, rv->signal_pair);
    assert(res == 0);
    set_non_blocking(rv->signal_pair[1]);
    uv_poll_init(rv->loop, &rv->signal_read_watcher, rv->signal_pair[1]);
    //rv->signal_read_watcher.data = rv;
    //uv_poll_init(rv->loop, &rv->signal_write_watcher, rv->signal_pair[1]);
    uv_poll_start(&rv->signal_read_watcher, UV_READABLE, signal_read_cb);
    //uv_poll_start(&rv->signal_write_watcher, UV_WRITABLE, signal_write_cb);

    pthread_mutex_init(&rv->io_mutex, NULL);
    res = pthread_create(&rv->io_thread, NULL, _run_io_thread, rv);
    assert(res == 0);

    return rv;
}

void
http_io_destroy(http_io** ctx_handle)
{
    assert(ctx_handle);
    http_io* ctx = *ctx_handle;
    assert(ctx);
    if (ctx) {
        close(ctx->signal_pair[0]);
        close(ctx->signal_pair[1]);
        uv_poll_stop(&ctx->signal_read_watcher);
        //uv_poll_stop(&ctx->signal_write_watcher);
        pthread_join(ctx->io_thread, NULL);
        pthread_mutex_destroy(&ctx->io_mutex);
        uv_loop_delete(ctx->loop);
        memset(ctx, 0, sizeof(*ctx));
    }
    free(*ctx_handle);
    *ctx_handle = NULL;
}


void
http_io_signal_get(http_io* ctx, int* fdp, http_io_signal_cb* cbp)
{
    *fdp = ctx->signal_pair[0];
    *cbp = _signal_cb;
}

// ----------------------------------------------------------------------------
