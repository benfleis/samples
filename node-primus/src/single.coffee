http = require 'http'
util = require 'util'

Primus = require 'primus'

#
# create single trivial primus namespace with single client
#

main = (opts) ->
    server = http.createServer()

    foo = new Primus server, { pathname: '/foo', close: false }

    foo.on 'connection', (spark) ->
        console.log 'spark.%s CONNECTION [%s:%s]', spark.id, spark.address.ip, spark.address.port

        spark.on 'data', (data) ->
            console.log 'spark.%s DATA: %s', spark.id, util.inspect data
            # calling w/ close: true kills the underlying socket.
            foo.end { close: true, timeout: 100 }

        spark.write 'Hello, client.'

    foo.on 'disconnection', (spark) -> console.log 'spark.%s DISCONNECTION', spark.id
    foo.on 'error', (err) -> console.error 'primus ERROR: %s', err.toString()

    # -------------------------------------------------------------------------

    clientCheck = () ->
        Socket = Primus.createSocket { pathname: '/foo' }
        socket = Socket 'ws://localhost:8080', {}

        # useful debugging snippet logs all event emissions
        #emit = socket.emit
        #socket.emit = () ->
        #    console.log ('socket.emit ' + (Array::slice.call arguments, 0).join ', ')
        #    emit.apply @, arguments

        socket.on 'open', () -> console.log 'client OPEN'
        socket.on 'end', () -> console.log 'client END'
        socket.on 'reconnecting', () -> console.log 'client RECONNECTING'
        socket.on 'data', (data) -> console.log 'client DATA %s', data.toString()

        socket.write { goo: 'balls' }


    server.listen 8080, clientCheck

# -----------------------------------------------------------------------------

if require.main is module
  main process.argv.slice 1
