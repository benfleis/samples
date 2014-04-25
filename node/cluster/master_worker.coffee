_ = require 'lodash'
cluster = require 'cluster'
http = require 'http'
util = require 'util'


logEmit = (eventEmitter, log=console.log, tag='') ->
    tag = if tag then tag += ' ' else ''
    emit = eventEmitter.emit
    eventEmitter.emit = () =>
        log '%s%s', tag, (Array::slice.call arguments).join ', '
        emit.apply eventEmitter, arguments

console.log 'running: ' + process.pid
if cluster.isMaster
    # spawn initial workers
    #logEmit cluster, null, 'MASTER'

    maybeRespawnWorker = (worker, code, signal) ->
        if not worker.suicide
            console.log 'Respawning worker'
            cluster.fork()

    numWorkers = 20
    for i in [0...numWorkers]
        cluster.fork()

    # re-create worker upon unnatural death
    cluster.on 'exit', maybeRespawnWorker

    cluster.on 'listening', (worker) ->
        console.log 'MASTER: worker[%s] listening', worker.process.pid
        setTimeout (whackAMole.bind null, worker), 1000

    whackAMole = (worker) ->
        console.log 'MASTER: sending \'detach\' to worker %s', worker.process.pid
        worker.send 'detach'

        worker.once 'message', (msg) ->
            console.log 'MASTER: worker[%s] response: %s', worker.process.pid, msg
            if msg == 'detached'
                console.log 'MASTER: disconnect()ing worker[%s]', worker.process.pid
                worker.disconnect()

                worker.once 'disconnect', () ->
                    console.log 'MASTER: \'disconnect\' from worker[%s]: %s', worker.suicide, worker.process.pid

else if cluster.isWorker
    httpServer = http.createServer()
    httpServer.listen 8080, '0.0.0.0'
    httpServer.on 'request', (req, resp) -> resp.end('yes\n')

    process.on 'message', (msg) ->
        console.log 'WORKER[%s]: message %s', process.pid, msg
        if msg == 'detach'
            httpServer.close () -> process.send 'detached'
