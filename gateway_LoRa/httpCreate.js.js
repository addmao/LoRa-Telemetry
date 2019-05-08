var http = require('http');
var mysql = require('mysql');
//var express = require('express');
//var app = express();

var conn = mysql.createConnection({
	host: 'localhost',
	user: 'root',
	password: 'zazaza123',
	database: 'water_level',
	insecureAuth: true
});


//var server = app.listen(8080, function(req, res) {
//	console.log('Creating http server');
//	//console.log(err);
//	conn.query('SELECT * FROM monitoring_system', [1,2], function(err,rows,fields){
//		res.set({'Content-Type': 'application/json'});
//		console.log(err);
//		//console.log(rows);
//		res.end(JSON.stringify(rows));
//		res.end();
//	})
//})

console.log('MySQL connection details ' + conn);

http.createServer(function(req, res) {
	console.log('Creating http server');
	//console.log(err);
	conn.query('SELECT * FROM monitoring_system', [1,2], function(err,rows,fields){
		//res.writeHead(200, {'Content-Type': 'application/json','Access-Control-Allow-Orgin': ['*']});
		//res.setHeader('Access')
			res.setHeader('Access-Control-Allow-Origin', '*');
		res.setHeader('Access-Control-Request-Method', '*');
	res.setHeader('Access-Control-Allow-Methods', 'OPTIONS, GET');
	res.setHeader('Access-Control-Allow-Headers', '*');

		console.log(err);
		//console.log(rows);
		res.end(JSON.stringify(rows));
		res.end();
	})
}).listen(9001);

//conn.query('SELECT * FROM water_level_data', (err, results, fields) => {
//	if(err) { return console.error(err.message); }
//	console.log(results);
//});

//conn.end();


