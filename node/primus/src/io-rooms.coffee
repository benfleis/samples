http = require 'http'
util = require 'util'

Primus = require 'primus.io'

# useful debugging snippet logs all event emissions
blue = '\u001b[34m'
green = '\u001b[32m'
log = () -> arguments[0] += '\u001b[0m'; console.log.apply null, arguments
teeEmit = (x, title) ->
    return
    emit = x.emit
    x.emit = () ->
        log '%s: %s', title, ((Array::slice.call arguments, 0).join ', ')
        emit.apply @, arguments


spawnServer = (opts) ->
    server = http.createServer()
    server.listen 8080, spawnClients

    primus = new Primus server
    primus.on 'connection', (spark) ->
        log blue + 'primus connection'

        spark.on 'join', (room) ->
            console.log 'join ' + room
            spark.join room

            debugger
            spark.on 'command', (data) ->
                debugger
                log '%sReceived command: %s', green, (util.inspect data)

        spark.on 'leave', (room) -> spark.leave room


spawnClients = () ->
    Socket = Primus.createSocket {}
    socket = Socket 'ws://localhost:8080'

   #socket.on 'open', (spark) ->
   #    log blue + 'socket connection'
   #    @send 'join', 'myRoom1'
   #    @send { room: 'myRoom1', 'command': { bingo: 'bango' } }

# -----------------------------------------------------------------------------

if require.main is module
  spawnServer()


