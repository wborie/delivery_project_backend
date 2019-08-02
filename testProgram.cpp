#include <iostream>

int main(int argc, const char* argv[]) {
	std::cout << argv[0] << " " << argc << std::endl;
	std::cout << "hello world!" << std::endl;
	if (argc == 3) {
		std::cout << argv[1] << " " << argv[2] << std::endl;
	}
}



// Input: A list of pairs of Locations: [{start: restaurant1, end: home1}, {start: restaurant2, end: home2}] and the map graph

// Output: A list of paths: [{roadSector1, roadSector2}, {roadSector3, roadSector4, roadSector5}]
// Output could be one path (if that's what's optimal) <-- when is it optimal to use a second driver?
// Scenario: Run dijsktra from res1 to home1. Run dijkstra from res2 to home2