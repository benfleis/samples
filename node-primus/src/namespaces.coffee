http = require 'http'
util = require 'util'

Primus = require 'primus'

#
# create 2 primus namespaces, /foo and /bar, on same server, and 2 clients
#

main = (opts) ->
    httpServer = http.createServer()

    fooServer = new Primus httpServer, { pathname: '/foo' }
    barServer = new Primus httpServer, { pathname: '/bar' }

    endCount = 2
    for server in [fooServer, barServer]
        tag = '[' + server.pathname + ']'
        do (server, tag) ->
            server.on 'connection', (spark) ->
                sparkTag = tag + '/spark.' + spark.id

                console.log '%s CONNECTION [%s:%s]', sparkTag, spark.address.ip, spark.address.port

                spark.on 'disconnection', (spark) -> console.log '%s DISCONNECTION', sparkTag
                spark.on 'end', () ->
                    if --endCount is 0
                        console.log 'All clients disconnected.  Exiting.'
                        server.end()

                spark.on 'data', (data) ->
                    console.log '%s DATA: %s', sparkTag, util.inspect data
                    spark.write 'received: ' + data.goo

        server.on 'error', (err) -> console.error '%s ERROR: %s', tag, err.toString()

    # -------------------------------------------------------------------------

    clientCheck = () ->
        createClient = (name) ->
            Socket = Primus.createSocket { pathname: '/' + name }
            socket = Socket 'ws://localhost:8080', {}
            socket.on 'open', () -> console.log '[client %s] OPEN', name
            socket.on 'end', () -> console.log '[client %s] END', name
            socket.on 'reconnecting', () -> console.log '[client %s] RECONNECTING', name
            socket.on 'data', (data) -> console.log '[client %s] DATA %s', name, data.toString()
            socket

        fooClient = createClient 'foo'
        barClient = createClient 'bar'

        fooClient.write { goo: 'foo' }
        barClient.write { goo: 'bar' }

        for client in [fooClient, barClient]
            do (client) ->
                client.on 'data', (data) ->
                    client.end()

    httpServer.listen 8080, clientCheck

# -----------------------------------------------------------------------------

if require.main is module
  main process.argv.slice 1
