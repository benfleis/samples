http = require 'http'
util = require 'util'

Primus = require 'primus'
PrimusRooms = require 'primus-rooms'

#
# what I want: a server and 2 clients, all connected to same room.
#

main = (opts) ->
    server = http.createServer()

    foo = new Primus server, { pathname: '/foo', plugin: { rooms: PrimusRooms } }

    # not follow example; what I want is to simply connect to a pathname/room,
    # and just go.
    debugger
    members = {}
    foo.on 'connection', (spark) ->
        console.log 'spark.%s CONNECTION [%s:%s]', spark.id, spark.address.ip, spark.address.port

        spark.on 'data', (data) ->
            console.log 'spark.%s DATA: %s', spark.id, util.inspect data
            debugger

            channel = null
            if data?.action == 'join'
                spark.join '#channel', () ->
                    members[spark.id] = true
                    channel = spark.room '#channel'
                    channel.write (util.format '#channel members: [%s]', (Object.keys members).join ', ')
            else if data?.action == 'leave'
                spark.leave '#channel', () ->
                    delete members[spark.id]
                    channel.write (util.format '#channel members: [%s]', (Object.keys members).join ', ')
            else
                if data?.msg and channel?.write
                    channel.write data.msg

            console.log 'channel is set? %s', !!channel


    foo.on 'disconnection', (spark) -> console.log 'spark.%s DISCONNECTION', spark.id
    foo.on 'error', (err) -> console.error 'primus ERROR: %s', err.toString()

    # -------------------------------------------------------------------------

    clientCheck = () ->
        createChannelClient = (name) ->
            Socket = Primus.createSocket { pathname: '/foo' }
            socket = Socket 'ws://localhost:8080', {}

            socket.on 'open', () -> console.log '%s OPEN', name
            socket.on 'end', () -> console.log '%s END', name
            socket.on 'reconnecting', () -> console.log '%s RECONNECTING', name
            socket.on 'data', (data) -> console.log '%s DATA %s', name, data.toString()
            socket.write { action: 'join', room: '#channel' }
            socket

        client1 = createChannelClient 'client1'
        client2 = createChannelClient 'client2'

        client1.write { room: '#channel', msg: 'from client1' }
        client2.on 'data', (data) ->
            if data?.msg == 'from client1'
                client2.write { room: '#channel', msg: 'i saw you, client1. -client2' }

    server.listen 8080, clientCheck

# -----------------------------------------------------------------------------

if require.main == module
  main process.argv.slice 1
