CFLAGS := -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -Werror -I../libuv/include
CFLAGS += -g -O0
LDFLAGS := -L. -lfriendly -L../libuv -luv -lpthread -framework Foundation -framework CoreServices
#LDFLAGS=../libuv/libuv.a

# XXX must make this work dynamically between mac/linux.  but i don't really
# want to learn auto* for this.
#ifeq (`uname s`, Darwin)
#LDFLAGS += -framework Foundation
#endif

cli_SRCS := cli_uv.c
cli_OBJS := $(cli_SRCS:.c=.o)
cli_EXES := $(cli_SRCS:.c=)

libfriendly_SRCS := http_io.c friendly.c
libfriendly_OBJS := $(libfriendly_SRCS:.c=.o)
libfriendly_ARC := libfriendly.a

all:	$(cli_EXES)

$(cli_EXES):	$(cli_OBJS) $(libfriendly_ARC)

$(libfriendly_ARC):	$(libfriendly_OBJS)
	$(AR) rcs $@ $^

clean:
	rm -rf $(libfriendly_OBJS) $(cli_OBJS) $(libfriendly_ARC) $(cli_EXES) $(libfriendly_OBJS:.o=.dSYM) $(cli_OBJS:.o=.dSYM)

.PHONY: clean
