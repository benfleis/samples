all:	pthread
	

clean:
	rm -rf pthread pthread.o pthread.dSYM

CFLAGS := -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -Wall -Werror -I../../third-party/libuv/include
LDFLAGS := -L../../third-party/libuv -luv -lpthread -framework Foundation -framework CoreServices
#LDFLAGS=../../third-party/libuv/libuv.a

# XXX must make this work dynamically between mac/linux.  but i don't really
# want to learn auto* for this.
#ifeq (`uname s`, Darwin)
#LDFLAGS += -framework Foundation
#endif
