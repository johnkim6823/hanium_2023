const express = require('express');
const cors = require("cors");
const mysql = require("mysql");
var net = require('net');
const app = express();
const PORT = 3001;

const db = mysql.createPool({
    host: "127.0.0.1",
    user: "hanium",
    password: "1234",
    database: "hanium"
});

app.use(cors({
    origin:"*",
    credentials: true,
    optionsSuccessStatus: 200,
}))

app.use(express.urlencoded({extended: true}));

app.listen(PORT, () => {
    console.log(`server running on port${PORT}`);
});

app.get("/api/2023_0417", (req,res) => {
    res.header("Access-Control-Allow-Origin", "*")
    const sqlQuery = "SELECT * FROM 2023_0417";
    db.query(sqlQuery, (err, result) => {
        res.send(result);
    })
})

function getConnection(connName){
    var client = net.connect({port: 8700, host:'localhost'}, function() {
      console.log(connName + ' Connected: ');
      console.log('   local = %s:%s', this.localAddress, this.localPort);
      console.log('   remote = %s:%s', this.remoteAddress, this.remotePort);
      this.setTimeout(500);
      this.setEncoding('utf8');
      this.on('data', function(data) {
        console.log(connName + " From Server: " + data.toString());
        this.end();
      });
      this.on('end', function() {
        console.log(connName + ' Client disconnected');
      });
      this.on('error', function(err) {
        console.log('Socket Error: ', JSON.stringify(err));
      });
      this.on('timeout', function() {
        console.log('Socket Timed Out');
      });
      this.on('close', function() {
        console.log('Socket Closed');
      });
    });
    return client;
  }

  function writeData(socket, data){
    var success = !socket.write(data);
    if (!success){
      (function(socket, data){
        socket.once('drain', function(){
          writeData(socket, data);
        });
      })(socket, data);
    }
  }
app.get("/api/hello", (req,res) => {
    var server = getConnection("hanium");
    writeData(server, "More Axes");
})