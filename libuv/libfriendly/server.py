#!/usr/bin/env python

##
# don't even do SimpleHTTPServer, just listen, and have a completely fixed
# response, also fixed in size, so that the client does *zero* parsing.  could
# chop the response and send it in varying byte lengths for partial buffer
# testing as well.
#

import hashlib
import re
import socket
import sys

def main(args):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('localhost', 6600))
    s.listen(1)

    # do nothing in parallel
    while True:
	(child_s, address) = s.accept()
	buf = ''
	sys.stderr.write('server: new connection\n')
	while True:
	    res = child_s.recv(1024)
	    sys.stderr.write('server: new buffer recv\'d\n')
	    #import ipdb; ipdb.set_trace()
	    if not res:	# received shutdown
		child_s.close()
		break
	    buf += res
	    index = buf.find('\r\n\r\n')
	    if index >= 0:
		sys.stderr.write('server: handling message\n')
		eor = index + 4
		req, buf = buf[:eor], buf[eor:]

		# now build response that is exactly 1024 bytes, including header
		digest = hashlib.sha224(req).hexdigest()
		resp = 'HTTP/1.1 200 OK\r\nContent-Length: 984\r\n\r\n'
		resp += digest
		resp += '\0' * (1024 - len(resp))

		child_s.send(resp)

if __name__ == '__main__':
    main(sys.argv)
