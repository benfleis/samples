// XXX temporary

#include "http_io.h"

int
main(int argc, const char* const* argv)
{
    http_io_t* http = http_io_create();
    http_io_destroy(&http);

    return 0;
}
