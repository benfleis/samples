http = require 'http'
util = require 'util'

Primus = require 'primus.io'

# useful debugging snippet logs all event emissions
blue = '\u001b[34m'
green = '\u001b[32m'
log = () -> arguments[0] += '\u001b[0m'; console.log.apply null, arguments
teeEmit = (x, title) ->
    emit = x.emit
    x.emit = () ->
        log '%s: %s', title, ((Array::slice.call arguments, 0).join ', ')
        emit.apply @, arguments


spawnServer = (opts) ->
    server = http.createServer()
    server.listen 8080, spawnClients

    primus = new Primus server
    #teeEmit primus, (blue + 'primus')
    primus.on 'connection', (spark) ->
        teeEmit spark, (blue + 'primus subscribed SPARK')
        spark.on 'subscribe', (channelName, channelSpark) ->
            log green + 'got SUBSCRIBE'
            #teeEmit channelSpark, (green + 'primus subscribed channelSPARK')
            channelSpark.on 'command', (data) ->
                log '%sReceived command: %s', green, util.inspect data

        spark.on 'unsubscribe', () ->
            # XXX somehow coming in before the commands
            log green + 'got UNSUBSCRIBE'
            primus.end()



spawnClients = () ->
    Socket = Primus.createSocket {}
    socket = Socket 'ws://localhost:8080'

    foo = socket.channel '#foo'
    #teeEmit foo, '#foo'

    foo.send 'command', { data: 'bar' }
    foo.send 'command', { data: 'bingo' }
    foo.end()

# -----------------------------------------------------------------------------

if require.main is module
  spawnServer()

