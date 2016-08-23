//
// index.js
//

var zmq = require('zmq'),
  TlsTerminate = require('./tls_terminate.js'),
  q = require('q'),
  spawn = require('child_process').spawn;

function Queue() {};
Queue.prototype.listen = function(port, cb) {
  var zsock = zmq.socket("router");
  return q.ninvoke(zsock, "bind", "tcp://*:" + port).then(function() {
    if ((cb) && (typeof(cb) == 'function')) cb(zsock);
    return zsock;
  }).catch(function(e) {
    console.log(e);
  });
}

var queue = new Queue();
var proxy = new TlsTerminate();


queue.listen(23401).then(function(zmtp) {
  return q.fcall(function() {
    // Backend is up, set up proxy...
    var server = proxy.listen({
      key: "./server_key.pem",
      cert: "./server_cert.pem",
      tcp: 23401,
      tcps: 23400
    });

    // Handle zmq.
    zmtp.on("message", function(rid, msg) {
      zmtp.send([rid, msg.toString().toUpperCase()]);
    });
    zmtp.on("error", function(e) {
      console.log(e);
    });
    return {
      zmtp: zmtp,
      server: server
    };
  });
}).then(function(servers) {
  // Backend started...  Spawn some tests...
  var deferred = q.defer();
  var client = spawn('../programs/uzmtp-client', ['tcp://127.0.0.1:23400', 'this is a test']);
  client.on('close', function() {
    //
  });
  client.stdout.on('data', function(data) {
    var result = data.toString();
    if (result == 'THIS IS A TEST\n') {
      console.log("test passed!");
      deferred.resolve(servers);
    } else {
      console.log("test failed!");
      deferred.reject(servers);
    }
  });
  return deferred.promise;
}).then(function(servers) {
  // Kill our processes.
  servers.server.close();
  servers.zmtp.close();
}).done();

//
//
//
