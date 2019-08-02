#ifndef GRAPH_H
#define GRAPH_H

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cfloat>
#include <queue>
#include <functional>
#include <iostream> // !!!!! Debugging only

template <typename Node>
class Graph {	
public:
	Graph(int inputCapacity);
	~Graph();
	void deleteNodeList();
	void insertNode(Node newNode);
	void increaseCapacity();
	std::string getNodeList();
	std::string getGraphString();
	void addEdge(Node firstNode, Node secondNode, double weight);
	void deleteEdges();
	std::vector<std::string> dijkstra(Node startNode);

private:
	struct NodeContainer;
	struct Edge {
		NodeContainer* from;
		NodeContainer* to;
		double weight;
	};
	struct NodeContainer {
		Node node;
		int nodeListIndex;
		bool set;
		std::vector<Edge*> edges;
	};
	
	int capacity;
	int size;
	int currentNodeListIndex;
	int filledSize;
	int numEdges;
	NodeContainer** nodeList;

	std::vector<Edge*> getOutgoingEdges(Node node);
	struct DijkstraNodeContainerComparator
	{
		double* distance;
		DijkstraNodeContainerComparator(double* initialDistance) {
			distance = initialDistance;
		}
		bool operator()(const NodeContainer* lhs, const NodeContainer* rhs)
		{
			return distance[lhs->nodeListIndex] > distance[rhs->nodeListIndex];
		}
	};
};

template <typename Node>
Graph<Node>::Graph(int inputCapacity) {
	size = 0;
	filledSize = 0;
	capacity = inputCapacity;
	currentNodeListIndex = 0;
	numEdges = 0;
	nodeList = new NodeContainer*[capacity];
	for(int i = 0; i < capacity; i++) {
		nodeList[i] = new NodeContainer;
		nodeList[i]->set = false;
	}
}

template <typename Node>
Graph<Node>::~Graph() {
	deleteEdges();
	deleteNodeList();
}

template <typename Node>
void Graph<Node>::deleteNodeList() {
	for(int i = 0; i < capacity; i++) {
		delete nodeList[i];
	}
	delete[] nodeList;
}

template <typename Node>
void Graph<Node>::deleteEdges() {
	for(int i = 0; i < capacity; i++) {
		std::vector<Edge*> currentNodeContainerEdges = nodeList[i]->edges;
		for(unsigned int j = 0; j < currentNodeContainerEdges.size(); j++) {
			delete currentNodeContainerEdges[j];
		}
	}
}

template <typename Node>
void Graph<Node>::insertNode(Node newNode) {
	if (size == capacity) increaseCapacity();
	nodeList[currentNodeListIndex]->set = true;
	nodeList[currentNodeListIndex]->node = newNode;
	nodeList[currentNodeListIndex]->nodeListIndex = currentNodeListIndex;
	size++;
	filledSize++;
	currentNodeListIndex++;
}

template <typename Node>
void Graph<Node>::increaseCapacity() {
	int newCapacity = capacity * 2;
	NodeContainer** newNodeList = new NodeContainer*[newCapacity];
	for(int i = 0; i < newCapacity; i++) {
		newNodeList[i] = nullptr;
	}
	for(int i = 0; i < capacity; i++) {
		newNodeList[i] = nodeList[i];
	}
	for(int i = capacity; i < newCapacity; i++) {
		newNodeList[i] = new NodeContainer;
		newNodeList[i]->set = false;
	}

	delete[] nodeList;
	nodeList = newNodeList;
	capacity = newCapacity;
}

template <typename Node>
std::string Graph<Node>::getNodeList() {
	std::string result = "";
	for(int i = 0; i < capacity; i++) {
		if (nodeList[i]->set) result += nodeList[i]->node + " | ";
	}
	return result;
}

template <typename Node>
std::string Graph<Node>::getGraphString() {
	std::string result = "";
	for(int i = 0; i < capacity; i++) {
		if (nodeList[i]->set) result += nodeList[i]->node + " : ";
		std::vector<Edge*> currentNodeContainerEdges = nodeList[i]->edges;
		for(unsigned int j = 0; j < currentNodeContainerEdges.size(); j++) {
			result += "[" + currentNodeContainerEdges[j]->from->node + ", " + currentNodeContainerEdges[j]->to->node + ", " + 
			std::to_string(currentNodeContainerEdges[j]->weight) + "]";
		}
		if (nodeList[i]->set) result += "\n";
	}
	return result;
}

template <typename Node>
void Graph<Node>::addEdge(Node firstNode, Node secondNode, double weight) { // first = from, second = to
	NodeContainer* firstNodeContainer = nullptr;
	NodeContainer* secondNodeContainer = nullptr;
	for(int i = 0; i < capacity; i++) {
		if (nodeList[i]->set) {
			if (nodeList[i]->node == firstNode) firstNodeContainer = nodeList[i];
			else if (nodeList[i]->node == secondNode) secondNodeContainer = nodeList[i];
		}
	}
	if (firstNodeContainer == nullptr || secondNodeContainer == nullptr) return; // To and from nodes aren't in the graph

	Edge* newEdge = new Edge;
	newEdge->from = firstNodeContainer;
	newEdge->to = secondNodeContainer;
	newEdge->weight = weight;
	firstNodeContainer->edges.push_back(newEdge);
	numEdges++;
}

template <typename Node>
std::vector<typename Graph<Node>::Edge*> Graph<Node>::getOutgoingEdges(Node currentNode) { // This is probably useless
	std::vector<Edge*> result;
	NodeContainer* currentNodeContainer = nullptr;
	for(int i = 0; i < capacity; i++) {
		if (nodeList[i]->set) {
			if (nodeList[i]->node == currentNode) currentNodeContainer = nodeList[i];
		}
	}
	if (currentNodeContainer == nullptr) return result;
	else return currentNodeContainer->edges;
}

#endif