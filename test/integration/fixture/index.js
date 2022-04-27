const zmq = require("zeromq");
const s = zmq.socket("router");
const exec = require("child_process").exec;
const PORT = 33558;
const command = process.argv[2];

console.log(`Fixture running (${PORT})`);

s.bind(`tcp://*:${PORT}`);
s.on("message", function toUpper(id, hello, world) {
  s.send([
    id,
    Buffer.from(hello).toString().toUpperCase(),
    Buffer.from(world).toString().toUpperCase(),
  ]);
  i++;
});

if (command) {
  console.log(`Executing command: ${command}`);
  const cp = exec(`${command}`, function (e, stdout, stderr) {
    if (e instanceof Error) {
      console.error(e);
      throw e;
    } else {
      s.close();
      console.log(stdout);
      console.error(stderr);
      process.exit(cp.exitCode);
    }
  });
}
