libev
=======

first build libev:

        git submodule update --init
        cd ../third-party/libev
        autoreconf -if && CFLAGS=-g ./configure && make

then you should be able to compile from within stdio with make.
