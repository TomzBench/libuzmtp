var zmq = require("zeromq");
var s = zmq.socket("router");
const PORT = 33558;

console.log(`Fixture running (${PORT})`);

s.bind(`tcp://*:${PORT}`);
s.on("message", function (id, hello, world) {
  s.send([
    id,
    Buffer.from(hello).toString().toUpperCase(),
    Buffer.from(world).toString().toUpperCase(),
  ]);
  s.close();
});
