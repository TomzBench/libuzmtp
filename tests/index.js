// index.js
//
// This script first creates a server for use as a test fixture for uzmtp 
// library.  When test fixture sockets are ready, we than spawn uzmtp-client
// applications with various parameters to test the library. uzmtp-client 
// connects to our test fixture. We compare output from uzmtp-client to verify
// if test passes.  When test is finished we close test fixture.
//
// (Test fixture is capitalize echo server.)

// TODO - Add more logging feedback.  Use config script for ports.

var zmq = require('zmq'),
  TlsTerminate = require('./tls_terminate.js'),
  q = require('q'),
  spawn = require('child_process').spawn;

// Trivial module to manage ZMQ socket...
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

// Proxy and ZMQ instances.
var queue = new Queue();
var proxy = new TlsTerminate();


// Begin test...
queue.listen(23401).then(function(zmtp) {
  // Create test fixture...
  return q.fcall(function() {
    var server = proxy.listen({
      key: "./server_key.pem",
      cert: "./server_cert.pem",
      tcp: 23401,
      tcps: 23400
    });

    // Echo server...
    zmtp.on("message", function(rid, msg) {
      zmtp.send([rid, msg.toString().toUpperCase()]);
    });
    zmtp.on("error", function(e) {
      console.log(e);
    });

    // Test context...
    return {
      zmtp: zmtp, // ZMQ port 
      server: server, // TLS proxy port
      exe: [
        ['../programs/uzmtp-client', 'tcp://127.0.0.1:23400', 'this is a test'],
        ['../programs/uzmtp-client', 'tcp://127.0.0.1:23400', 'this is a test']
      ]
    };
  });
}).then(function(ctx) {
  // Test fixture is up... Spawn our client test application.
  var deferred = q.defer();
  ctx.results = [];
  (function _test(idx) {
    ctx.results[idx] = {
      response: '',
      error: ''
    }
    var client = spawn('valgrind', ctx.exe[idx]);
    client.on('close', function() {
      if (++idx < ctx.exe.length) _test(idx);
      else deferred.resolve(ctx);
      //deferred.resolve(ctx);
    });
    client.stdout.on('data', function(data) {
      //ctx.response += data.toString();
      ctx.results[idx].response += data.toString();
    });
    client.stderr.on('data', function(data) {
      //ctx.error += data.toString();
      ctx.results[idx].error += data.toString();
    });
  })(0);
  return deferred.promise;
}).then(function(ctx) {
  ctx.results.forEach(function(obj, idx) {
    console.log(
      "\nVALGRIND REPORT:\n" +
      "===============\n" +
      "%s\n" +
      "ECHO REPORT:\n" +
      "===============\n" +
      "%s\n" +
      "%s",
      ctx.results[idx].error, ctx.exe[idx][2], ctx.results[idx].response);
  });
  // Kill our processes.
  ctx.server.close();
  ctx.zmtp.close();
}).done();

//
//
//
