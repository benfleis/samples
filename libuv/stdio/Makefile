all:	stdio_poll

clean:
	rm -rf stdio_poll stdio_poll.o stdio.dSYM

CFLAGS := -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -Wall -Werror -I../libuv/include
LDFLAGS := -L../libuv -luv -framework Foundation -framework CoreServices
#LDFLAGS=../libuv/libuv.a

