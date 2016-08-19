//
// index.js
//

var zmq = require('zmq'),
  TlsTerminate = require('./tls_terminate.js'),
  q = require('q');

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
  // Backend is up, set up proxy...
  proxy.listen({
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
}).done();
