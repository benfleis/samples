CXXFLAGS := -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -Werror -I../../third-party/include -I../../third-party/libuv/include
CXXFLAGS += -g -O0
LDFLAGS := -L. -lfriendly -L../../third-party/libuv -luv -lpthread -framework Foundation -framework CoreServices -lstdc++
#LDFLAGS=../../third-party/libuv/libuv.a

# XXX must make this work dynamically between mac/linux.  but i don't really
# want to learn auto* for this.
#ifeq (`uname s`, Darwin)
#LDFLAGS += -framework Foundation
#endif

cli_SRCS := cli_uv.cc
cli_OBJS := $(cli_SRCS:.cc=.o)
cli_EXES := $(cli_SRCS:.cc=)

libfriendly_SRCS := http_io.cc friendly.cc
libfriendly_OBJS := $(libfriendly_SRCS:.cc=.o)
libfriendly_ARC := libfriendly.a

all:	$(cli_EXES)

$(cli_EXES):	$(cli_OBJS) $(libfriendly_ARC)

$(libfriendly_ARC):	$(libfriendly_OBJS)
	$(AR) rcs $@ $^

clean:
	rm -rf $(libfriendly_OBJS) $(cli_OBJS) $(libfriendly_ARC) $(cli_EXES)

.PHONY: clean
