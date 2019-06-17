var express = require('express');
var MongoClient = require('mongodb').MongoClient;
var bodyParser = require('body-parser');

var app = express();
app.use(bodyParser.json());
const PORT = 4000;
const MONGO_URL = "mongodb://localhost:27017"

app.get('/', (req, res) => {
	res.send("Hit root endpoint");
});

app.post('/intersection', async (req, res) => {
	const client = await MongoClient.connect(MONGO_URL, {useNewUrlParser: true});
	if(!client) {
		res.status(400).send({status: `Error connecting to the Database: ${err}`});
	} else if (!req.body.roadSectors || (req.body.x === undefined) || (req.body.y === undefined) || 
		(req.body.isEndPoint === undefined)) {
		res.status(400).send({status: "Error inserting Intersection: Not all required parameters were set."});
	} else {
		const db = client.db("delivery_project");
		let result = await db.collection("Intersections").insertOne({roadSectors: req.body.roadSectors, x: req.body.x, 
			y: req.body.y, isEndPoint: req.body.isEndPoint});
		if (result) {
			res.send({status: "Successfully inserted Intersection"});
		} else {
			res.status(400).send({status: "Error inserting Intersection."});
		}
	}
});

app.post('/roadSector', async (req, res) => {
	const client = await MongoClient.connect(MONGO_URL, {useNewUrlParser: true});
	if (!client) {
		res.status(400).send({status: `Error connecting to the Database: ${err}`});
	} else if (!req.body.roadName || !req.body.roadSectorId  || !req.body.roadSectorLength || 
		(req.body.addressMinNumber === undefined) || (req.body.addressMaxNumber === undefined)) {
		res.status(400).send({status: "Error inserting RoadSector: Not all required parameters were set."});
	} else {
		const db = client.db("delivery_project");
		let result = await db.collection("RoadSectors").insertOne({roadName: req.body.roadName, roadSectorId: req.body.roadSectorId, 
			roadSectorLength: req.body.roadSectorLength, addressMinNumber: req.body.addressMinNumber,
			addressMaxNumber: req.body.addressMaxNumber});
		if (result) {
			res.send({status: "Successfully inserted RoadSector."});
		} else {
			res.status(400).send({status: "Error inserting RoadSector."});
		}
	}
});

// TODO: validate which items are strings/numbers
app.post('/location', async (req, res) => {
	console.log('test');
	const client = await MongoClient.connect(MONGO_URL, {useNewUrlParser: true});
	if (!client) {
		res.status(400).send({status: `Error connecting to the Database: ${err}`});
	} else if (!req.body.name || !req.body.type || !req.body.roadName || !req.body.addressNumber ||
	 (req.body.x === undefined) || (req.body.y === undefined)) {
	 	console.log(req.body.name, req.body.type, req.body.roadName, req.body.addressNumber, req.body.x, req.body.y)
		res.status(400).send({status: "Error inserting Location: Not all required parameters were set."});
	} else {
		const db = client.db("delivery_project");
		const roadSector = await db.collection("RoadSectors").findOne({roadName: req.body.roadName, addressMinNumber: {$lte: req.body.addressNumber},
			addressMaxNumber: {$gte: req.body.addressNumber}});
		if (!roadSector) {
			res.status(400).send({status: "Error inserting Location: The provided location's address couldn't be mapped to a roadSector."});
		} else {
			const result = await db.collection("Locations").insertOne({name: req.body.name, type: req.body.type, 
				roadName: req.body.roadName, addressNumber: req.body.addressNumber, roadSectorId: roadSector.roadSectorId, 
				x: req.body.x, y: req.body.y});
			if (result) {
				res.send({status: "Successfully inserted Location."});
			} else {
				res.status(400).send({status: "Error inserting Location."});
			}
		}
	}		
});

// Takes in a list of pairs of Locations, returns a list of paths
app.post('/deliveries', (req, res) => {
	const execFile = require('child_process').execFile;
	
	const process = execFile("./deliveryRoutes", [])
});

app.get('/testProcess', (req, res) => {
	console.log("In testProcess");
	let execFile = require('child_process').execFile;
	let child = execFile("./testProgram", ["first arg", "second arg"], (err, stdout, stderr) => {
		console.log("Program output: ");
		console.log(stdout);
	})
	res.send("initiated testProcess");

})

app.post('/testEndpoint', (req, res) => {
	res.send({result: "success!!", first: req.body.first});
})


app.listen(PORT, () => console.log(`Server is listening on port ${PORT}`));


// Example of using Mongo
// app.post('/addIntersection', (req, res) => {
// 	MongoClient.connect(MONGO_URL, (err, client) => {
// 		if (err) throw `Error connecting to the database: ${err}`;
// 		else console.log("Successfully connected to the database.");
// 		var db = client.db("delivery_project")
// 		db.collection("Nodes").insertOne({a: 1}, (err, result) => {
// 			console.log("Result", result);
// 			res.send("Done inserting intersection into the database.");
// 			client.close();
// 		});
// 	});
// });