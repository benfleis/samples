// This program is demo for using pthreads with libev.
// Try using Timeout values as large as 1.0 and as small as 0.000001
// and notice the difference in the output

// (c) 2009 debuguo
// (c) 2013 enthusiasticgeek for stack overflow
// Free to distribute and improve the code. Leave credits intact

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "uv.h"

static pthread_mutex_t lock;
static uint64_t timeout;
static uv_timer_t timeout_watcher;
static int timeout_count = 0;

static uv_async_t async_watcher;
static int async_count = 0;

static uv_loop_t* loop;
static uv_loop_t* loop_for_async_thread;

void*
run_other_thread(void* args)
{
    printf("Running secondary: uv_run(loop_for_async_thread, 0);\n");
    uv_run(loop_for_async_thread, UV_RUN_DEFAULT);
    return NULL;
}

static void
async_cb(uv_async_t* w, int revents)
{
    //fprintf(stderr, "async ready\n");
    pthread_mutex_lock(&lock);     // Don't forget locking
    ++async_count;
    printf("async = %d, timeout = %d\n", async_count, timeout_count);
    pthread_mutex_unlock(&lock);   // Don't forget unlocking
}

static void
timeout_cb(uv_timer_t* w, int revents) // Timer callback function
{
    //fprintf(stderr, "timeout\n");
    uv_async_send(&async_watcher);

    pthread_mutex_lock(&lock);     // Don't forget locking
    ++timeout_count;
    pthread_mutex_unlock(&lock);   // Don't forget unlocking
    w->repeat = timeout;
    uv_timer_again(&timeout_watcher); // Start the timer again.
}

int
main(int argc, const char* const* argv)
{
    if (argc < 2) {
        fprintf(stderr, "Timeout value missing.\n%s <timeout_ms>\n", argv[0]);
        return -1;
    }
    timeout = atoi(argv[1]);

    // Initialize pthread
    pthread_mutex_init(&lock, NULL);
    pthread_t thread;

    // loop runs under main thread, loop_for_async_thread runs in the pthread
    loop = uv_default_loop();
    loop_for_async_thread = uv_loop_new();

    // This block is specifically used pre-empting thread (i.e. temporary
    // interruption and suspension of a task, without asking for its
    // cooperation, with the intention to resume that task later.)
    //
    // This takes into account thread safety
    uv_async_init(loop_for_async_thread, &async_watcher, async_cb);
    //uv_async_start(loop_for_async_thread, &async_watcher);
    pthread_create(&thread, NULL, run_other_thread, NULL);

    // Non repeating timer. The timer starts repeating in the timeout callback
    // function
    uv_timer_init(loop, &timeout_watcher);
    uv_timer_start(&timeout_watcher, timeout_cb, timeout, 0);

    // now wait for uvents to arrive
    printf("Running primary: uv_run(loop, 0);\n");
    uv_run(loop, 0);
    // Wait on threads for execution
    pthread_join(thread, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}
