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

#define offset_of(TYPE, MEMBER) ((uint64_t) &((TYPE *)0)->MEMBER)

// ----------------------------------------------------------------------------

struct request {
    // request bits
    uint32_t len;
    uint8_t* msg;

    // response bits
    struct {
        http_io_response_cb cb;
        int status;
        uint32_t len;
        uint8_t* msg;
    } response;
};

typedef std::map<uint32_t, request> request_by_id_t;
struct http_io {
    int signal_pair[2];     // socketpair fds; [0]=main_thread, [1]=io_thread
#define _MAIN_THREAD        0
#define _IO_THREAD          1
    pthread_t io_thread;
    uv_loop_t* loop;

    uv_poll_t signal_io_thread_read_watcher;
    //uv_poll_t signal_write_watcher;

    //uv_read_t data_read_watcher;
    //uv_write_t data_write_watcher;

    http_io_response_cb response_cb;

    request_by_id_t requests;
    pthread_mutex_t requests_mutex;

    // ------------------------------------------------------------------------

    const char* ip;
    uint16_t port;
    uv_tcp_t http;
    uv_connect_t http_connect;
    uv_shutdown_t shutdown;
};

// ----------------------------------------------------------------------------
// prototypes for the major callbacks, so we can order how we want
//

void signal_read_cb(uv_poll_t* watcher, int status, int revents);

void _http_connect_cb(uv_connect_t* watcher, int status);
void _http_read_cb(uv_stream_t* http, ssize_t res, uv_buf_t buf);
void _http_shutdown_cb(uv_shutdown_t* shutdown, int status);
void _http_close_cb(uv_handle_t* handle);
uv_buf_t _http_new_buf(uv_handle_t* _ignored, size_t suggested_len);


// ----------------------------------------------------------------------------

static int
_signal_cb(http_io* ctx, uint64_t cookie)
{
    //uint32_t cmd = cookie >> 32;
    uint32_t id = cookie & 0xffffffff;

    // fetch context from hash table
    // call user callback on returned data, if specified.
    typedef std::map<uint32_t, request> request_by_id_t;
    pthread_mutex_lock(&ctx->requests_mutex);
    request_by_id_t::iterator ri = ctx->requests.find(id);
    assert(ri != ctx->requests.end());
    pthread_mutex_unlock(&ctx->requests_mutex);

    return 0;
}

static void*
_run_io_thread(void* args)
{
    http_io* ctx = static_cast<http_io*>(args);
    uv_run(ctx->loop, UV_RUN_DEFAULT);
    return NULL;
}

static int
set_non_blocking(int fd)
{
  int val = fcntl(fd, F_GETFL, 0);
  return val == -1 ? -1 : fcntl(fd, F_SETFL, val | O_NONBLOCK);
}

// ----------------------------------------------------------------------------

uv_buf_t
_http_new_buf(uv_handle_t* _ignored, size_t suggested_len)
{
    return uv_buf_init(new char[suggested_len], suggested_len);
}

void
_http_read_cb(uv_stream_t* http, ssize_t res, uv_buf_t buf)
{
    uv_loop_t* loop = http->loop;
    http_io* ctx = static_cast<http_io*>(loop->data);

    if (res < 0) {
        if (uv_last_error(loop).code == UV_EOF) {
            fprintf(stderr, "  uv_read_stop(http);\n");
            uv_read_stop(http);
        }
        else
            assert(false);
    }
    else {
        fprintf(stderr, "_http_read_cb(..., %lu) ->\n%s\n\n", res, buf.base);
        if (buf.base)
            delete buf.base;

        // signal, callback, ...
        // XXX

        // remove outstanding request; if no reqs, stop http socket read evt
        pthread_mutex_lock(&ctx->requests_mutex);
        ctx->requests.erase(ctx->requests.begin());
        if (ctx->requests.empty()) {
            fprintf(stderr, "  uv_read_stop(http);\n");
            uv_read_stop(http);
        }
        pthread_mutex_unlock(&ctx->requests_mutex);
    }
}

void
_http_connect_cb(uv_connect_t* watcher, int status)
{
    fprintf(stderr, "_http_connect_cb(%d)\n", status);
    http_io* ctx = container_of(watcher, http_io, http_connect);
    assert((void*)watcher->handle == (void*)&ctx->http);

    // once we're connected, attach the signal read handler
    uv_poll_start(&ctx->signal_io_thread_read_watcher, UV_READABLE, signal_read_cb);
}

void
_http_shutdown_cb(uv_shutdown_t* shutdown, int status)
{
    fprintf(stderr, "_http_shutdown_cb(%d)\n", status);
    http_io* ctx = container_of(shutdown, http_io, shutdown);
    uv_close((uv_handle_t*)&ctx->http, _http_close_cb);
}

void
_http_close_cb(uv_handle_t* handle)
{
    // NOOP
    fprintf(stderr, "_http_close_cb()\n");
}

// ----------------------------------------------------------------------------

struct http_write_ctx {
    http_io* ctx;
    uint32_t id;
    uv_write_t req;
    uv_buf_t buf;
};

static void
http_write_cb(uv_write_t* req, int status)
{
    http_write_ctx* ctx = container_of(req, http_write_ctx, req);
    delete ctx->buf.base;
    delete ctx;

    // create read context
    // XXX
}

void
signal_read_cb(uv_poll_t* watcher, int status, int revents)
{
    fprintf(stderr, "signal_read_cb(...) {\n");
    http_io* ctx = container_of(watcher, http_io, signal_io_thread_read_watcher);
    uint32_t id;
    int32_t len = read(ctx->signal_pair[_IO_THREAD], &id, sizeof(id));

    if (len < 0) {
        if (uv_last_error(ctx->loop).code == UV_EOF) {
            fprintf(stderr, "  uv_poll_stop(signal);\n");
            uv_poll_stop(watcher);
        }
        else {
            fprintf(stderr, "  read(%d, buf): ERROR=%s\n", ctx->signal_pair[_IO_THREAD], strerror(errno));
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                //fprintf(stderr, "read(%d, buf) -> %s\n", ctx->signal_pair[_IO_THREAD], strerror(errno));
                uv_poll_stop(watcher);
            }
        }
    }
    else {
        // whoo hoo!  got a message.  snag it, send it, be happy
        assert(len == 4);
        fprintf(stderr, "  read(%d, buf): ID=%u\n", ctx->signal_pair[_IO_THREAD], id);
        pthread_mutex_lock(&ctx->requests_mutex);
        request_by_id_t::iterator reqi = ctx->requests.find(id);
        assert(reqi != ctx->requests.end());
        pthread_mutex_unlock(&ctx->requests_mutex);

        http_write_ctx* req = new http_write_ctx();
        req->buf = uv_buf_init(new char[reqi->second.len], reqi->second.len);
        memcpy(req->buf.base, reqi->second.msg, reqi->second.len);
        uv_read_start((uv_stream_t*)&ctx->http, _http_new_buf, _http_read_cb);
        uv_write(&req->req, (uv_stream_t*)&ctx->http, &req->buf, 1, http_write_cb);
    }
    fprintf(stderr, "}\n");
}

//static
void
signal_write_cb(uv_poll_t* watcher, int status, int wevents)
{
    fprintf(stderr, "signal_write_cb(...) { }\n");
}


// ----------------------------------------------------------------------------

http_io*
http_io_create(const char* ip, uint16_t port)
{
    http_io* rv = new http_io();
    assert(rv);

    fprintf(stderr, "http_io_create(%s, %d)\n", ip, port);

    // easy stuff
    rv->ip = ip;
    rv->port = port;
    rv->loop = uv_loop_new();
    rv->loop->data = rv;                // save http_io ctx for later
    assert(rv->loop);
    pthread_mutex_init(&rv->requests_mutex, NULL);

    // setup socket pair, then setup read/write checks for io_thread side
    int res = socketpair(PF_LOCAL, SOCK_DGRAM, 0, rv->signal_pair);
    assert(res == 0);
    set_non_blocking(rv->signal_pair[_IO_THREAD]);
    uv_poll_init(rv->loop, &rv->signal_io_thread_read_watcher, rv->signal_pair[_IO_THREAD]);
    //uv_poll_init(rv->loop, &rv->signal_write_watcher, rv->signal_pair[_IO_THREAD]);

    // connect to endpoint
    uv_tcp_init(rv->loop, &rv->http);
    struct sockaddr_in dst = uv_ip4_addr(ip, port);
    uv_tcp_connect(&rv->http_connect, &rv->http, dst, _http_connect_cb);

    // finish by instantiating run thread
    res = pthread_create(&rv->io_thread, NULL, _run_io_thread, rv);
    assert(res == 0);
    return rv;
}


//
// Politely disconnect from underlying HTTP socket; meaning that we accept no
// new sends, and when there are no outstanding requests,
//
//static void
//http_io_close(http_io* ctx)
//{
//}

static void
_print_watcher(uv_handle_t* handle, void* arg)
{
// define UV__HANDLE_ACTIVE    0x40
    if (handle->flags & 0x40)
        fprintf(stderr, "waiting on handle of type %d\n", handle->type);
}

static void
_print_remaining_watchers(http_io* ctx)
{
    uv_walk(ctx->loop, _print_watcher, 0);
}

void
http_io_destroy(http_io* ctx)
{
    assert(ctx);
    if (ctx) {
        sleep(2);
        fprintf(stderr, "http_io_destroy(...) {\n");
        //uv_poll_stop(&ctx->signal_io_thread_read_watcher);
        close(ctx->signal_pair[_MAIN_THREAD]);
        //uv_poll_stop(&ctx->signal_write_watcher);
        // XXX evil, crosses thread line.
        //if (ctx->http.loop)
        uv_shutdown(&ctx->shutdown, (uv_stream_t*)&ctx->http, _http_shutdown_cb);
        for (int i = 0; i < 10; i++) {
            _print_remaining_watchers(ctx);
            sleep(1);
        }
        pthread_join(ctx->io_thread, NULL);
        pthread_mutex_destroy(&ctx->requests_mutex);
        uv_loop_delete(ctx->loop);
        memset(ctx, 0, sizeof(*ctx));
        free(ctx);
        fprintf(stderr, "}\n");
    }
}

void
http_io_signal_get(http_io* ctx, int* fdp, http_io_signal_cb* cbp)
{
    *fdp = ctx->signal_pair[_MAIN_THREAD];
    *cbp = _signal_cb;
}

// ----------------------------------------------------------------------------

//
// send means: put actual request into shared queue, send 1 byte on the socket
// to wake up the io_thread
//
int
http_io_send(http_io* ctx, void* query, uint32_t query_len, http_io_response_cb cb)
{
    // package and "enqueue" request
    static char fmt[] = "GET %s HTTP/1.1\r\n\r\n";
    static uint32_t id;

    // build up the http request line
    const int line_len = sizeof(fmt) + query_len - 1;   // -2 from %s, +1 for nul term
    char* line = new char[line_len];            // XXX not deleted
    snprintf(line, line_len, "GET %s HTTP/1.1\r\n\r\n", query);

    // package and insert into request map, then write actual signaling
    // message w/ id
    struct request req = { line_len, (uint8_t*)line, { cb } };
    pthread_mutex_lock(&ctx->requests_mutex);
    ctx->requests.insert(request_by_id_t::value_type(id, req));
    pthread_mutex_unlock(&ctx->requests_mutex);
    // now write to the pipe so that sibling thread can get it
    // XXX quick and dirty, but should normally use events
    int res = write(ctx->signal_pair[_MAIN_THREAD], (void*)&id, sizeof(id));
    assert(res == sizeof(id));

    return 0;
}
