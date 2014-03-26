http = require 'http'
util = require 'util'

Primus = require 'primus.io'

#
# create single trivial primus namespace with single client
#

main = (opts) ->
    server = http.createServer()

    foo = new Primus server, { pathname: '/foo' }

    foo.on 'connection', (spark) ->
        console.log 'spark.%s CONNECTION [%s:%s]', spark.id, spark.address.ip, spark.address.port

        spark.on 'channel1', (data) ->
            console.log 'spark.%s #channel1: %s', spark.id, util.inspect data
            spark.send 'channel1', 'response'
            spark.send 'log', 'channel1 xaction'

        spark.on 'log', (data) ->
            console.log 'spark.%s #log: %s', spark.id, util.inspect data

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
        #socket.on 'data', (data) -> console.log 'client DATA %s', data.toString()

        #socket.write { goo: 'balls' }
        counter = 2
        socket.on 'open', () ->
            socket.send 'channel1', 'got here.'
            socket.on 'channel1', (data) ->
                console.log 'client #channel1 %s', data.toString()
                socket.send 'log', 'client saw ' + data.toString()
            socket.on 'log', (data) ->
                console.log 'client #log %s', data.toString()
                foo.end()

    server.listen 8080, clientCheck

# -----------------------------------------------------------------------------

if require.main is module
  main process.argv.slice 1
