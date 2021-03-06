#include <stdint.h>
#include <sys/types.h>

extern "C" {

typedef struct http_io http_io_t;

http_io_t* http_io_create(const char* ip, uint16_t port);
void http_io_destroy(http_io_t*);

//
// Signaling is presently simple: select on fd, read a uint64_t
// from the datagram socket, then call the given callback with that uint64_t.
// if the callback returns 0, continue to select on it.  otherwise decode the
// error via http_io_error(...) [or something like it.  NOT IMPLEMENTED.]
//
typedef int (*http_io_signal_cb)(http_io*, uint64_t);

void http_io_signal_get(http_io_t* ctx, int* fdp, http_io_signal_cb* cbp);

//
// After an HTTP GET, we store the response, and write to the signal socket.
// When the user reads that socket datagram and calls the callback, we will in
// turn call the registered response below.
//
typedef void (*http_io_response_cb)(int status, void* response, uint32_t len);

void http_io_register_response_callback_default(http_io_t*, http_io_response_cb);

int http_io_send(http_io_t*, void* request, uint32_t len, http_io_response_cb cb);

}
