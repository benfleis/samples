stdio
=======

first test of libuv usage -- just takes simple inputs, counts the chars, then
spits out how many are read.  both stdin and stdout are handled via libuv
callbacks, and even respected in the way we exit.
