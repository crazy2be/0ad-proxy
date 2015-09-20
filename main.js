var express = require("express");
var app = express();

var request = require("request");
// http://stackoverflow.com/questions/9920208/expressjs-raw-body
app.use (function(req, res, next) {
    var data='';
    req.setEncoding('utf8');
    req.on('data', function(chunk) {
       data += chunk;
    });

    req.on('end', function() {
        req.body = data;
        next();
    });
});
app.all("/*", function(req, res) {
 //console.log("Got request", req);
 var url = unescape(req.path).slice(1);
 console.log("Requesting url", url)

 res.send('');

 var opts = {
  url: 'https://0ad.firebaseio.com/' + url,
  method: req.method,
  body: req.body,
 };
 console.log(opts);
 request(opts, function(err, response, body) {
	console.log(response.body);
  //res.send(response.body);
 })
});

app.listen(2500, function() {
 console.log("Started server");
});
