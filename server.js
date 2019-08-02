var express = require('express');
var MongoClient = require('mongodb').MongoClient;
var bodyParser = require('body-parser');
const priority_queue = require('./helpers').priority_queue;
const Graph = require('./helpers').Graph;
const objectValuesAreEqual = require('./helpers').objectValuesAreEqual;

var app = express();
app.use(bodyParser.json());
const PORT = 4000;
const MONGO_URL = "mongodb://localhost:27017"

app.get('/', (req, res) => {
	res.send("Hit root endpoint");
});

app.post('/intersection', async (req, res) => {
	const client = await MongoClient.connect(MONGO_URL, { useNewUrlParser: true });
	if (!client) {
		res.status(400).send({ status: `Error connecting to the Database: ${err}` });
	} else if (!req.body.roadSectors || (req.body.x === undefined) || (req.body.y === undefined) ||
		(req.body.isEndPoint === undefined)) {
		res.status(400).send({ status: "Error inserting Intersection: Not all required parameters were set." });
	} else {
		const db = client.db("delivery_project");
		let result = await db.collection("Intersections").insertOne({
			roadSectors: req.body.roadSectors, x: req.body.x,
			y: req.body.y, isEndPoint: req.body.isEndPoint
		});
		if (result) {
			res.send({ status: "Successfully inserted Intersection" });
		} else {
			res.status(400).send({ status: "Error inserting Intersection." });
		}
	}
});

app.post('/roadSector', async (req, res) => {
	const client = await MongoClient.connect(MONGO_URL, { useNewUrlParser: true });
	if (!client) {
		res.status(400).send({ status: `Error connecting to the Database: ${err}` });
	} else if (!req.body.roadName || !req.body.roadSectorId || !req.body.roadSectorLength ||
		(req.body.addressMinNumber === undefined) || (req.body.addressMaxNumber === undefined)) {
		res.status(400).send({ status: "Error inserting RoadSector: Not all required parameters were set." });
	} else {
		const db = client.db("delivery_project");
		let result = await db.collection("RoadSectors").insertOne({
			roadName: req.body.roadName, roadSectorId: req.body.roadSectorId,
			roadSectorLength: req.body.roadSectorLength, addressMinNumber: req.body.addressMinNumber,
			addressMaxNumber: req.body.addressMaxNumber
		});
		if (result) {
			res.send({ status: "Successfully inserted RoadSector." });
		} else {
			res.status(400).send({ status: "Error inserting RoadSector." });
		}
	}
});

// TODO: validate which items are strings/numbers
app.post('/location', async (req, res) => {
	console.log('test');
	const client = await MongoClient.connect(MONGO_URL, { useNewUrlParser: true });
	if (!client) {
		res.status(400).send({ status: `Error connecting to the Database: ${err}` });
	} else if (!req.body.name || !req.body.type || !req.body.roadName || !req.body.addressNumber ||
		(req.body.x === undefined) || (req.body.y === undefined)) {
		console.log(req.body.name, req.body.type, req.body.roadName, req.body.addressNumber, req.body.x, req.body.y)
		res.status(400).send({ status: "Error inserting Location: Not all required parameters were set." });
	} else {
		const db = client.db("delivery_project");
		const roadSector = await db.collection("RoadSectors").findOne({
			roadName: req.body.roadName, addressMinNumber: { $lte: req.body.addressNumber },
			addressMaxNumber: { $gte: req.body.addressNumber }
		});
		if (!roadSector) {
			res.status(400).send({ status: "Error inserting Location: The provided location's address couldn't be mapped to a roadSector." });
		} else {
			const result = await db.collection("Locations").insertOne({
				name: req.body.name, type: req.body.type,
				roadName: req.body.roadName, addressNumber: req.body.addressNumber, roadSectorId: roadSector.roadSectorId,
				x: req.body.x, y: req.body.y
			});
			if (result) {
				res.send({ status: "Successfully inserted Location." });
			} else {
				res.status(400).send({ status: "Error inserting Location." });
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
	res.send({ result: "success!!", first: req.body.first });
});

app.post('/getRoutes', (req, res) => {
	if (req.body.map === undefined || req.body.requests === undefined) {
		return res.status(400).send({ status: "Error getting routes: Not all required parameters were set." });
	} else {
		return res.send({routes: bruteForceRouteOptimize(req.body.map, req.body.requests)});
	}
});

app.listen(PORT, () => console.log(`Server is listening on port ${PORT}`));

function bruteForceRouteOptimize(map, requests) {
	const graph = new Graph();
	map.nodes.forEach(node => {
		graph.addVertex(node);
	})
	map.edges.forEach(edge => {
		graph.addEdge(edge.start, edge.end, edge.weight);
	})

	let potentialCombinedRequests = [];
	requests.forEach(request => {
		requests.forEach(otherRequest => {
			if (otherRequest.id === request.id) return;
			else if (withinCloseRange(graph, request.start, otherRequest.start)
				&& withinCloseRange(graph, request.end, otherRequest.end)) {
				let alreadyCombined = false;
				potentialCombinedRequests.forEach(combinedRequest => {
					if (objectValuesAreEqual(request, combinedRequest.secondRequest) &&
						objectValuesAreEqual(otherRequest, combinedRequest.firstRequest)) {
						alreadyCombined = true;
					}
				})
				if (!alreadyCombined) {
					potentialCombinedRequests.push({firstRequest: request, secondRequest: otherRequest});
				}
			}
		})
	})

	potentialCombinedRequests = potentialCombinedRequests.map(combinedRequest => {
		let shortestPath = findShortestCombinedPath(graph, combinedRequest.firstRequest.start, combinedRequest.secondRequest.start, 
			combinedRequest.firstRequest.end, combinedRequest.secondRequest.end);
		let shortestPathLength = getTotalPathWeight(graph, shortestPath);
		if (shortestPath.length !== 0) {
			return {...combinedRequest, shortestPath: shortestPath, shortestPathLength: shortestPathLength}
		} else {
			return combinedRequest;
		}
	})

	potentialCombinedRequests = potentialCombinedRequests.sort((first, second) => {
		if (first.shortestPathLength > second.shortestPathLength) {
			return 1;
		} else if (first.shortestPathLength < second.shortestPathLength) {
			return -1;
		} else {
			return 0;
		}
	})

	let determinedRequestIds = [];
	let routes = [];
	
	for(let i = 0; i < potentialCombinedRequests.length; i++) {
		if (!determinedRequestIds.includes(potentialCombinedRequests[i].firstRequest.id) && 
			!determinedRequestIds.includes(potentialCombinedRequests[i].secondRequest.id)) {
			determinedRequestIds.push(potentialCombinedRequests[i].firstRequest.id);
			determinedRequestIds.push(potentialCombinedRequests[i].secondRequest.id);
			let routeObject = {
				includedRequests: [potentialCombinedRequests[i].firstRequest.id, potentialCombinedRequests[i].secondRequest.id],
				route: potentialCombinedRequests[i].shortestPath
			}
			routes.push(routeObject);
		}
	}
	requests.forEach(request => {
		if (!determinedRequestIds.includes(request.id)) {
			determinedRequestIds.push(request.id);
			let routeObject = {
				includedRequests: [request.id],
				route: dijkstra(graph, request.start, request.end)
			}
			routes.push(routeObject);
		}
	})
	return routes;
}

const CLOSENESS_RANGE = 5;
function withinCloseRange(graph, firstNode, secondNode) {
	let shortestPathBetweenTwoNodes1 = dijkstra(graph, firstNode, secondNode);
	let shortestPathBetweenTwoNodes2 = dijkstra(graph, secondNode, firstNode);
	let firstPathWeight = getTotalPathWeight(graph, shortestPathBetweenTwoNodes1);
	let secondPathWeight = getTotalPathWeight(graph, shortestPathBetweenTwoNodes2);

	// Check if no path exists between the two nodes
	if (shortestPathBetweenTwoNodes1.length === 0 && shortestPathBetweenTwoNodes2.length === 0) return false;
	else if ((firstPathWeight <= CLOSENESS_RANGE && shortestPathBetweenTwoNodes1.length !== 0) ||
		(secondPathWeight <= CLOSENESS_RANGE && shortestPathBetweenTwoNodes2.length !== 0)) {
		return true;
	}
	else return false;
}

// Takes a path array of nodes within the graph and returns the weight of the path
function getTotalPathWeight(graph, path) {
	let totalPathWeight = 0;
	for (let i = 0; i < path.length - 1; i++) {
		totalPathWeight += graph.getEdges(path[i]).filter(edge => {
			return edge.end === path[i + 1];
		})[0].weight;
	}
	return totalPathWeight;
}

// expects graph of type Graph, startNode of type string, and endNode of type string, returns path array or [] if no path
function dijkstra(graph, startNode, endNode) {
	let queue = new priority_queue();
	let distances = {};
	let predecessors = {};
	graph.getVertices().forEach(node => {
		distances[node] = (node === startNode) ? 0 : Infinity;
		queue.push(node, distances[node]);
	})

	while (!queue.isEmpty()) {
		let currentNode = queue.pop();
		graph.getEdges(currentNode).forEach(edge => {
			const alternativePath = distances[currentNode] + edge.weight;
			if (alternativePath < distances[edge.end]) {
				distances[edge.end] = alternativePath;
				predecessors[edge.end] = currentNode;
				queue.push(edge.end, alternativePath);
				// queue.update_priority(edge.end, alternativePath);
			}
		})
	}

	currentNode = endNode;
	if (predecessors[endNode] === undefined) return [];
	else {
		let path = [];
		let count = 0;
		while (currentNode !== undefined && currentNode !== startNode) {
			path.push(currentNode);
			currentNode = predecessors[currentNode];
			count++;
			if (count === 100) break;
		}
		path.push(currentNode);
		path.reverse();
		return path;
	}
}

function findShortestCombinedPath(graph, start1, start2, end1, end2) {
	let paths = [];
	paths.push(determineCombinedPath(graph, start1, start2, end1, end2));
	paths.push(determineCombinedPath(graph, start1, start2, end2, end1));
	paths.push(determineCombinedPath(graph, start2, start1, end1, end2));
	paths.push(determineCombinedPath(graph, start2, start1, end2, end1));
	let pathWeights = []
	for(let i = 0; i < paths.length; i++) {
		pathWeights.push(getTotalPathWeight(graph, paths[i]));
	}
	let min = Infinity;
	let minIndex = 0;
	for(let i = 0; i < paths.length; i++) {
		if (paths[i].length !== 0 && pathWeights[i] < min) {
			min = pathWeights[i];
			minIndex = i;
		}
	}
	return paths[minIndex];
}

function determineCombinedPath(graph, first, second, third, fourth) {
	let path = [];
	let pathPossible = true;
	let result1 = dijkstra(graph, first, second);
	if (result1.length === 0) pathPossible = false;
	if (pathPossible) {
		path = path.concat(result1);
		let result2 = dijkstra(graph, second, third);
		if (result2.length === 0) pathPossible = false;
		if (pathPossible) {
			result2.shift(); // get rid of repeated first node
			path = path.concat(result2);
			let result3 = dijkstra(graph, third, fourth);
			if (result3.length === 0) pathPossible = false;
			if (pathPossible) {
				result3.shift(); // get rid of repeated first node
				path = path.concat(result3);
			}
		}
	}
	return pathPossible ? path : [];
}




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