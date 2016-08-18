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
  queue.listen(23401).then(function(sock) {
    tls.createServer({
      key: fs.readFileSync("./server_key.pem"),
      cert: fs.readFileSync("./server_cert.pem")
    }, function(s) {
      s.on("data", function(data) {
        c.write(data, "binary");
      });
      s.on("error", function(error) {
        console.log(error);
      });
      s.on("end", function() {
        console.log("server terminated");
      });

      var c = net.connect({
        port: 23401
      });
      c.on("data", function(data) {
	s.write(data, "binary");
      });
      c.on("error", function(e) {
        console.log(error);
      });
      c.on("end", function() {
        console.log("client terminated");
      });

    }).listen(23400);

    sock.on("message", function(rid, msg) {
      console.log(msg.toString());
    });
    sock.on("error", function(e) {
      console.log(e);
    });
  }).done();
}
//
//
//
