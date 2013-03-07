libfriendly
===========

libfriendly is a demonstration of a library that can support both thread-based
and event-based asynchronous paradigms, as well as a synchronous "direct"
paradigm.

Let's imagine there being 3 layers (there is of course more internal
stratification):

- *high level*: implements trivial, synchronous (blocking) API, a la `send(...)`
  and `receive(...)`
- *mid level*: provides 2 options: thread based and event based
- *low level*: implements actual send and receives, asynchronously; actual
  implementation could be threaded or event based, doesn't matter

libfriendly intends to first explore the dynamics and API of implementing a
low level API based on libuv, and a mid level that exports sockets (file
descriptors) for library-neutral event based.

contents
========

libfriendly will initially consist of 3 files:

- `http_io.c` (_low level_): low level, 2-threaded, libuv based http socket tester
- `friendly.c` (_mid level_): implement socket/event based api
- `cli_*.c` implements (_high level_): cli test program(s), using various methods to interact with mid level api
