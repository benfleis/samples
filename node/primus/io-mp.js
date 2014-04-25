var Primus, blue, green, http, log, spawnClients, spawnServer, teeEmit, util;

http = require('http');
util = require('util');
Primus = require('primus.io');

blue = '\u001b[34m';
green = '\u001b[32m';
log = function() {
  arguments[0] += '\u001b[0m';
  console.log.apply(null, arguments);
};

teeEmit = function(x, title) {
  var emit;
  emit = x.emit;
  x.emit = function() {
    log('%s: %s', title, (Array.prototype.slice.call(arguments, 0)).join(', '));
    emit.apply(this, arguments);
  };
};

spawnServer = function(opts) {
  var primus, server;
  server = http.createServer();
  server.listen(8080, spawnClients);
  primus = new Primus(server);
  primus.on('connection', function(spark) {
    spark.on('subscription', function(channelSpark) {
      console.log('yay!  dynamic subscriptions!')
      channelSpark.on('command', function() {
        // do stuff.
      });
    });
  });
};

spawnClients = function() {
  var Socket, foo, socket;
  Socket = Primus.createSocket({});
  socket = Socket('ws://localhost:8080');
  foo = socket.channel('#foo');
  foo.on('connection', function() {
    foo.send('command', { data: 'bar' });
  }
};

if (require.main === module) {
  spawnServer();
}
