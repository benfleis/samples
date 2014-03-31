http = require 'http'
util = require 'util'

Primus = require 'primus.io'


teeEmit = (x, title) ->
    emit = x.emit
    x.emit = () ->
        log '%s: %s', title, ((Array::slice.call arguments, 0).join ', ')
        emit.apply @, arguments


spawnServer = (opts) ->
    server = http.createServer()
    server.listen 8080, spawnClients

    primus = new Primus server
    control = primus.channel '#control'
    channels = control: control

    #primus.on 'connection', (spark) -> teeEmit spark, (blue + 'primus spark')

    # manually setup control, whose only current purpose is 'join' msgs
    control.on 'connection', (spark) ->
        log '%s#control: CONNECTION %s [%s:%s]', green, spark.id, spark.address.ip, spark.address.port
        spark.on 'join', joinChannel
    control.on 'disconnection', (spark) -> log 'spark.%s DISCONNECTION', spark.id
    control.on 'error', (err) -> console.error 'primus ERROR: %s', err.toString()

    # dynamic data channels, spawned from join message above
    joinChannel = (channelName) ->
        log '%s#control: JOIN %s', green, channelName
        @send 'joined', channelName       # XXX breaks without an ack.  not sure why.
        c = primus.channel channelName
        channels[channelName] = c
        c.on 'connection', (spark) ->
            log '%s%s: CONNECTION', blue, channelName
        c.on 'disconnection', (spark) -> log '%s%s DISCONNECTION', blue, channelName
        c.on 'error', (err) -> console.error '%s%s ERROR: %s', blue, channelName, err.toString()

        c.on 'bingo', () -> @send 'bango'


spawnClients = () ->
    Socket = Primus.createSocket {}
    socket = Socket 'ws://localhost:8080'
    teeEmit socket, 'socket'

    control = socket.channel '#control'
    control.send 'join', '#foo'
    teeEmit control, '#control CLIENT'
    control.on 'joined', (data) ->
        console.log 'CLIENT JOINED #foo'
        foo = socket.channel '#foo'
        foo.send 'bingo'
        foo.on 'connection', (spark) ->
            debugger
            log '#foo CONNECTION'
            teeEmit foo, '#foo'
            setTimeout (() -> foo.send 'event', 'message'), 1000
            foo.on 'foo', (data) -> log '#foo data %s', data.toString()

        foo.on 'bango', () -> console.log 'got it.'

# -----------------------------------------------------------------------------

if require.main is module
  spawnServer()

