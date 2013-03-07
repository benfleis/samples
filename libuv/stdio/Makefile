all:	stdio_poll

clean:
	rm -rf stdio_poll stdio_poll.o stdio.dSYM

CFLAGS := -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -Wall -Werror -I../../third-party/libuv/include
LDFLAGS := -L../../third-party/libuv -luv -framework Foundation -framework CoreServices
#LDFLAGS=../../third-party/libuv/libuv.a

