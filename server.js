var express = require('express');
var http = require('http');
var fs = require('fs');
var exec = require('child_process').exec;

require("console-stamp")(console, "mm/dd HH:MM:ss");

// Get url of the current National Geographic image of the day
function getUrl(schedule) {
  // Will schedule to run 24 hours from now
  if (schedule === true) {
    setInterval(function() { getUrl(false) }, 24 * 60 * 60 * 1000);
  }

  var url = 'http://natgeoapi.herokuapp.com/api/dailyphoto';

  http.get(url, function(res) {
    var body = '';

    res.on('data', function(chunk) {
      body += chunk;
    });

    res.on('end', function() {
      var response = JSON.parse(body);
      console.log('Got url: ' + response.src);
      downloadImage('http:' + response.src);
    });

  }).on('error', function(e) {
    console.log("Got error: ", e);
  });
}

// Download the given image
function downloadImage(imageUrl) {
  var file = fs.createWriteStream("public/tempImage.jpg");
  var request = http.get(imageUrl, function(response) {
    response.pipe(file);
    file.on('finish', function() {
      file.close();
      console.log('Downloaded file');
      convertImage();
    });
  });
}

// Convert image to 64bit color for pebble time
function convertImage() {
  console.log('Converting the image')
  var cmd = "convert 'public/tempImage.jpg' \
    -adaptive-resize '144x168^' \
    -gravity center -extent 144x168 \
    -fill '#FFFFFF00' -opaque none \
    -dither FloydSteinberg \
    -remap pebble_colors_64.gif \
    -define png:compression-level=9 -define png:compression-strategy=0 \
    -define png:exclude-chunk=all \
    'public/tempImage.png'";

  exec(cmd, function(error, stdout, stderr) {
    // command output is in stdout
  });
}



/* Scheduling and downloading of images */

// Start the download and don't schedule to run on first get
// Set a timeout for the next day, and then schedule for 24 hours later
getUrl(false);
var now = new Date();
var night = new Date(
  now.getFullYear(),
  now.getMonth(),
  now.getDate() + 1, // the next day, ...
  0, 0, 0 // ...at 00:00:00 hours
);
var nextDay = night.getTime() - now.getTime();
setTimeout(function() { getUrl(true) }, nextDay);


/* Setting up of the server itself */

// Set up the file server
var app = express();

// Serve files in the public directory
app.use(express.static('public'));

var server = app.listen(5000, function () {
  var host = server.address().address;
  var port = server.address().port;
  console.log('App listening at http://%s:%s', host, port);
});
