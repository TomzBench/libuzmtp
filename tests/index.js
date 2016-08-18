//
// index.js
//

var tls = require('tls'),
  zmq = require('zmq'),
  fs = require('fs');

(function() {
  if (module.parent) return;
  start();
})();

function start() {
  var server = tls.createServer({
    key: fs.readFileSync("./server_key.pem"),
    cert: fs.readFileSync("./server_cert.pem")
  }, function(s) {
    s.on("data", function(data) {
      console.log(data);
    });
    s.on("error", function(e) {
      console.log(e);
    });
    s.on("end", function() {
      console.log("end");
    });
  }).listen(23400);
}
