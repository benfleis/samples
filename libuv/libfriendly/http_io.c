#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>

#include "uv.h"

#include "http_io.h"


// ----------------------------------------------------------------------------

struct http_io {
    int signal_pair[2];     // fds for communication socketpair; [0] == main thread, [1] == io_thread
    pthread_t io_thread;
    pthread_mutex_t io_mutex;
    uv_loop_t* loop;

    http_io_response_cb response_cb;
};

// ----------------------------------------------------------------------------

static int _signal_cb(uint64_t index64)
{
    //uint32_t index = index64 & 0xffffffff;

    // fetch context from hash table
    // call user callback on returned data, if specified.
    //

    return 0;
}

// ----------------------------------------------------------------------------

uv_timer_t to;
void _dummy_cb(uv_timer_t *x, int y) {}

// ----------------------------------------------------------------------------

static void*
_run_io_thread(void* args)
{
    fprintf(stderr, "_run_io_thread(...) {\n");
    http_io_t* ctx = args;

    uv_timer_init(ctx->loop, &to);
    uv_timer_start(&to, _dummy_cb, 10, 0);

    uv_run(ctx->loop, UV_RUN_DEFAULT);

    fprintf(stderr, "} // _run_io_thread\n");
    return NULL;
}


// ----------------------------------------------------------------------------

http_io_t*
http_io_create(void)
{
    http_io_t* rv = calloc(sizeof(*rv), 1);
    assert(rv);
    rv->loop = uv_loop_new();
    assert(rv->loop);

    int res = socketpair(PF_LOCAL, SOCK_DGRAM, 0, rv->signal_pair);
    assert(res == 0);

    pthread_mutex_init(&rv->io_mutex, NULL);
    res = pthread_create(&rv->io_thread, NULL, _run_io_thread, rv);
    assert(res == 0);

    return rv;
}

void
http_io_destroy(http_io_t** ctx_handle)
{
    assert(ctx_handle);
    http_io_t* ctx = *ctx_handle;
    assert(ctx);
    if (ctx) {
        close(ctx->signal_pair[0]);
        close(ctx->signal_pair[1]);
        pthread_join(ctx->io_thread, NULL);
        pthread_mutex_destroy(&ctx->io_mutex);
        uv_timer_stop(&to);     // remove to get SEGV
        uv_loop_delete(ctx->loop);
        memset(ctx, 0, sizeof(*ctx));
    }
    free(*ctx_handle);
    *ctx_handle = NULL;
}


void
http_io_signal_get(http_io_t* ctx, int* fdp, http_io_signal_cb* cbp)
{
    *fdp = ctx->signal_pair[0];
    *cbp = _signal_cb;
}

// ----------------------------------------------------------------------------
