var zmq = require("zeromq");
var s = zmq.socket("router");

s.bind("tcp://*:33558");
s.on("message", function (id, hello, world) {
  s.send([
    id,
    Buffer.from(hello).toString().toUpperCase(),
    Buffer.from(world).toString().toUpperCase(),
  ]);
  s.close()
});
