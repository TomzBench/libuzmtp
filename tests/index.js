//
// index.js
//

var tls = require('tls'),
  zmq = require('zmq'),
  q = require('q'),
  net = require('net'),
  fs = require('fs');

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

start();

function start() {
  var queue = new Queue();
  queue.listen(23401).then(function(zmtp) {
    tls.createServer({
      key: fs.readFileSync("./server_key.pem"),
      cert: fs.readFileSync("./server_cert.pem")
    }, function(s) {

      // connect to our zmq backend and pip tls<->zmq
      var c = net.connect({
        port: 23401
      });
      s.pipe(c);
      c.pipe(s);
    }).listen(23400);

    zmtp.on("message", function(rid, msg) {
      console.log(rid + " " + msg.toString());
      zmtp.send([rid, msg.toString().toUpperCase()]);
    });
    zmtp.on("error", function(e) {
      console.log(e);
    });
  }).done();
}
//
//
//
