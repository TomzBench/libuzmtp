//
// tls_terminate.js
//
// Simple module for prototyping.  Production enviornment options for 
// TLS terminating could be:
// https://github.com/indutny/bud
// https://github.com/haproxy/haproxy
// https://github.com/varnish/hitch
//

var tls = require('tls'),
  fs = require('fs'),
  net = require('net');

function TlsTerminate() {};
TlsTerminate.prototype.listen = function listen(options) {
  return tls.createServer({
    key: fs.readFileSync(options.key),
    cert: fs.readFileSync(options.cert)
  }, function(s) {
    var c = net.connect({
      port: options.tcp
    });
    s.pipe(c);
    c.pipe(s);
  }).listen(options.tcps);
}

var terminate = module.exports = exports = TlsTerminate;
