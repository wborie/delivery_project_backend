#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

int HEADER_SIZE = 54;
int PIXEL_MAP_WIDTH = 11;
int PIXEL_MAP_HEIGHT = 11;
int DIRECTIONS_MAP_SIZE = 7; // Must be an odd number
int DIRECTIONS_MAP_RADIUS = DIRECTIONS_MAP_SIZE / 2;

unsigned int* getPixels();
void findDirectionsMap(unsigned int* pixelMap, int row, int col, int initialRow, int initialCol, bool* visited, 
	unsigned int* directionsMap);
void setNumDirections(unsigned int* pixelMap, int row, int col, unsigned int* directionsMap);

int main(int argc, const char* argv[]) {

	if (argc != 2) {
		std::cout << "Incorrect number of arguments: format should be ./imageParcer <filename>" << std::endl;
		return 1;
	}

	std::ifstream inputFile(argv[1]);

	char fileIdentifier[2];
	char c;
	inputFile.get(c);
	fileIdentifier[0] = c;
	inputFile.get(c);
	fileIdentifier[1] = c;
	if (fileIdentifier[0] != 'B' || fileIdentifier[1] != 'M') {
		std::cout << "File is not of type bmp." << std::endl;
		return 1;
	}
	std::cout << "File type: " << fileIdentifier[0] << fileIdentifier[1] << std::endl;

	union {
		char charRepresentation[4];
		unsigned int intRepresentation;
	} fileSize;
	for(int i = 0; i < 4; i++) {
		inputFile.get(fileSize.charRepresentation[i]);
	}
	std::cout << "File size: " << fileSize.intRepresentation << " bytes" << std::endl;; // Prints file size as # Bytes

	for(int i = 0; i < 4; i++) { // Read 4 more bytes
		inputFile.get();
	}

	union {
		char charRepresentation[4];
		unsigned int intRepresentation;
	} bitMapAddress;
	for(int i = 0; i < 4; i++) {
		inputFile.get(bitMapAddress.charRepresentation[i]);
	}
	std::cout << "Bit Map Address starts at: " << bitMapAddress.intRepresentation << std::endl;

	union {
		char charRepresentation[4];
		unsigned int intRepresentation;
	} headerSize;
	for(int i = 0; i < 4; i++) {
		inputFile.get(headerSize.charRepresentation[i]);
	}
	std::cout << "Header size: " << headerSize.intRepresentation << " bytes" << std::endl;
	if (headerSize.intRepresentation != 40) {
		std::cout << "This header may not be supported. Continuing..." << std::endl;
	}

	union {
		char charRepresentation[4];
		int intRepresentation;
	} width;
	for(int i = 0; i < 4; i++) {
		inputFile.get(width.charRepresentation[i]);
	}
	std::cout << "Image width: " << width.intRepresentation << " pixels" << std::endl;; // Prints width in pixels

	union {
		char charRepresentation[4];
		int intRepresentation;
	} height;
	bool heightNegative = false;
	for(int i = 0; i < 4; i++) {
		inputFile.get(height.charRepresentation[i]);
	}
	if (height.intRepresentation < 0) { // Height is negative (unsupported file format)
		heightNegative = true;
		height.intRepresentation *= -1;
	}
	std::cout << "Image height: " << height.intRepresentation << " pixels" << std::endl;; // Prints height in pixels
	if (heightNegative) {
		std::cout << "Image height is negative." << std::endl;
	} else {
		std::cout << "Image height is positive." << std::endl;
	}

	for(int i = 0; i < 2; i++) { // Read 2 more bytes
		inputFile.get();
	}

	union {
		char charRepresentation[2];
		short int intRepresentation;
	} bitsPerPixel;
	for(int i = 0; i < 2; i++) {
		inputFile.get(bitsPerPixel.charRepresentation[i]);
	}
	short int bytesPerPixel = bitsPerPixel.intRepresentation / 8;
	std::cout << "Bits per Pixel: " << bitsPerPixel.intRepresentation << " bits (" << 
		bytesPerPixel << " bytes)" << std::endl;; // Prints bits/bytes per pixel
		//40 or 124
	for(int i = 0; i < headerSize.intRepresentation - 16; i++) { // Read 24 more bytes to finish reading the header
		inputFile.get();
	}

	unsigned int size = bytesPerPixel * width.intRepresentation * height.intRepresentation;
	unsigned char* dataMap = new unsigned char[size];
	for(int i = 0; i < size; i++) {
		dataMap[i] = inputFile.get();
	}

	inputFile.close();

	if (!heightNegative) { // Flip data because negative Height = start bottom-left, work left-to-right, then upwards
		unsigned char* dataMapCopy = new unsigned char[size];
		for(int i = 0; i < size; i++) {
			dataMapCopy[i] = dataMap[i];
		}
		for(int row = 0; row < height.intRepresentation; row++) {
			for(int col = 0; col < (width.intRepresentation * bytesPerPixel); col++) {
				dataMap[((row * width.intRepresentation * bytesPerPixel)) + col] = 
					dataMapCopy[(((height.intRepresentation - row - 1) * width.intRepresentation * bytesPerPixel) + col)];
			}
		}
		delete[] dataMapCopy;
	}

	for(int row = 0; row < height.intRepresentation; row++) { // Change BGR format to RGB
		for(int col = 0; col < width.intRepresentation; col++) {
			unsigned int blueIndex = bytesPerPixel * ((row * width.intRepresentation) + col);
			unsigned int redIndex = (bytesPerPixel * ((row * width.intRepresentation) + col)) + 2;
			unsigned char swap = dataMap[blueIndex];
			dataMap[blueIndex] = dataMap[redIndex];
			dataMap[redIndex] = swap;
		}
	}

	// for(int row = 0; row < height.intRepresentation; row++) {
	// 	std::cout << " ================= Row: " << row << " =================" << std::endl;
	// 	for(int col = 0; col < (width.intRepresentation * bytesPerPixel); col += bytesPerPixel) {
	// 		int rowOffset = row * (width.intRepresentation * bytesPerPixel);
	// 		std::cout << " Col: " << (col / 4) << " (";
	// 		std::cout << (int)dataMap[rowOffset + col] << ","; // red
	// 		std::cout << (int)dataMap[rowOffset + col + 1] << ","; // green
	// 		std::cout << (int)dataMap[rowOffset + col + 2] << ")" << std::endl; // blue
	// 	}
	// }

	unsigned int numPixels = width.intRepresentation * height.intRepresentation;
	unsigned int* distanceMap = new unsigned int[numPixels];
	int redBase = 0; // TODO: calculate these from the image OR have it specified by command line
	int greenBase = 0;
	int blueBase = 0;
	for(int row = 0; row < height.intRepresentation; row++) {
		for(int col = 0; col < (width.intRepresentation * bytesPerPixel); col += bytesPerPixel) {
			int rowOffset = row * (width.intRepresentation * bytesPerPixel);
			int redValue = (int)dataMap[rowOffset + col];
			int greenValue = (int)dataMap[rowOffset + col + 1];
			int blueValue = (int)dataMap[rowOffset + col + 2];
			int distance = sqrt(pow(redValue - redBase, 2) + pow(greenValue - greenBase, 2) + 
				pow(blueValue - blueBase, 2));
			distanceMap[(row * width.intRepresentation) + (col / bytesPerPixel)] = distance;
		}
	}

	unsigned int* pixelMap = getPixels();
	for(int row = 0; row < PIXEL_MAP_HEIGHT; row++) {
		for(int col = 0; col < PIXEL_MAP_WIDTH; col++) {
			if (pixelMap[(row * PIXEL_MAP_WIDTH) + col]) { // Pixel is true
				bool* visited = new bool[121];
				for(int i = 0; i < 121; i++) {
					visited[i] = 0;
				}
				unsigned int* directionsMap = new unsigned int[121];
				for(int i = 0; i < 121; i++) {
					directionsMap[i] = 0;
				}
				findDirectionsMap(pixelMap, row, col, row, col, visited, directionsMap);
				setNumDirections(pixelMap, row, col, directionsMap);
				std::cout << row << " " << col << std::endl;
				for(int i = 0; i < DIRECTIONS_MAP_SIZE; i++) {
					for(int j = 0; j < DIRECTIONS_MAP_SIZE; j++) {
						std::cout << directionsMap[(i * DIRECTIONS_MAP_SIZE) + j] << " ";
					}
					std::cout << std::endl;
				}
				std::cout << std::endl << "=============" << std::endl << std::endl;
				delete[] visited;
				delete[] directionsMap;
			}
		}
	}
	std::cout << "Final pixelMap" << std::endl;
	for(int i = 0; i < PIXEL_MAP_HEIGHT; i++) {
		for(int j = 0; j < PIXEL_MAP_WIDTH; j++) {
			std::cout << pixelMap[(i * PIXEL_MAP_WIDTH) + j] << " ";
		}
		std::cout << std::endl;
	}
	delete[] pixelMap;


	// for(int row = 0; row < height.intRepresentation; row++) {
	// 	for(int col = 0; col < width.intRepresentation; col++) {

	// 	}
	// }

	// for(int row = 0; row < height.intRepresentation; row++) {
	// 	for(int col = 0; col < width.intRepresentation; col++) {
	// 		int distance = distanceMap[(row * width.intRepresentation) + col];
	// 		if (distance == 0) {
	// 			std::cout << "*" << " ";
	// 		} else if (distance < 10) { // 1 char
	// 			std::cout << "*" << " ";
	// 		} else if (distance < 100) { // 2 chars
	// 			std::cout << "*" << " ";
	// 		} else if (distance < 200) { // 3 chars
	// 			std::cout << "!" << " ";
	// 		} else if (distance < 1000) {
	// 			std::cout << " " << " ";
	// 		}
	// 	}
	// 	std::cout << std::endl << std::endl;
	// }


	delete[] dataMap;
}

// Returns nothing, modifies the directionsMap, setting borders to 1 if a path exists from initial to that border pixel
void findDirectionsMap(unsigned int* pixelMap, int row, int col, int initialRow, int initialCol, bool* visited, 
	unsigned int* directionsMap) {
	// std::cout << row << " " << col << " " << initialRow << " " << initialCol << std::endl;
	if (row < 0 || col < 0 || row >= PIXEL_MAP_HEIGHT || col >= PIXEL_MAP_WIDTH || 
		row < initialRow - DIRECTIONS_MAP_RADIUS || row > initialRow + DIRECTIONS_MAP_RADIUS || 
		col < initialCol - DIRECTIONS_MAP_RADIUS || col > initialCol + DIRECTIONS_MAP_RADIUS) {
		return;
	} else if (visited[((DIRECTIONS_MAP_RADIUS + (row - initialRow)) * DIRECTIONS_MAP_SIZE) + 
			(DIRECTIONS_MAP_RADIUS + (col - initialCol))]) {
		return;
	} else if ((row == initialRow - DIRECTIONS_MAP_RADIUS || row == initialRow + DIRECTIONS_MAP_RADIUS
		|| col == initialCol - DIRECTIONS_MAP_RADIUS || col == initialCol + DIRECTIONS_MAP_RADIUS) && 
		pixelMap[(row * PIXEL_MAP_WIDTH) + col]) {
		visited[((DIRECTIONS_MAP_RADIUS + (row - initialRow)) * DIRECTIONS_MAP_SIZE) + 
			(DIRECTIONS_MAP_RADIUS + (col - initialCol))] = 1;
		directionsMap[((DIRECTIONS_MAP_RADIUS + (row - initialRow)) * DIRECTIONS_MAP_SIZE) + 
			(DIRECTIONS_MAP_RADIUS + (col - initialCol))] = 1;
	} else if ((row == initialRow - DIRECTIONS_MAP_RADIUS || row == initialRow + DIRECTIONS_MAP_RADIUS 
		|| col == initialCol - DIRECTIONS_MAP_RADIUS || col == initialCol + DIRECTIONS_MAP_RADIUS) && 
		!pixelMap[(row * PIXEL_MAP_WIDTH) + col]) {
		visited[((DIRECTIONS_MAP_RADIUS + (row - initialRow)) * DIRECTIONS_MAP_SIZE) + 
			(DIRECTIONS_MAP_RADIUS + (col - initialCol))] = 1;
	} else {
		visited[((DIRECTIONS_MAP_RADIUS + (row - initialRow)) * DIRECTIONS_MAP_SIZE) + 
			(DIRECTIONS_MAP_RADIUS + (col - initialCol))] = 1;
		if (row - 1 >= 0 && pixelMap[((row - 1) * PIXEL_MAP_WIDTH) + col]) {
			findDirectionsMap(pixelMap, row - 1, col, initialRow, initialCol, visited, directionsMap);
		}
		if (row + 1 < PIXEL_MAP_HEIGHT && pixelMap[((row + 1) * PIXEL_MAP_WIDTH) + col]) {
			findDirectionsMap(pixelMap, row + 1, col, initialRow, initialCol, visited, directionsMap);
		}
		if (col - 1 >= 0 && pixelMap[(row * PIXEL_MAP_WIDTH) + col - 1]) {
			findDirectionsMap(pixelMap, row, col - 1, initialRow, initialCol, visited, directionsMap);
		}
		if (col + 1 < PIXEL_MAP_WIDTH && pixelMap[(row * PIXEL_MAP_WIDTH) + col + 1]) {
			findDirectionsMap(pixelMap, row, col + 1, initialRow, initialCol, visited, directionsMap);
		}
	}
	return;
}

// Takes directionMap and sets numDirections on pixelMap
void setNumDirections(unsigned int* pixelMap, int row, int col, unsigned int* directionsMap) {
	bool lastVisitedTrue = false;
	unsigned int numDirections = 0;
	for(int i = 0; i < DIRECTIONS_MAP_SIZE; i++) {
		if (directionsMap[i]) {
			if (!lastVisitedTrue) {
				lastVisitedTrue = true;
				numDirections++;
			}
		} else {
			lastVisitedTrue = false;
		}
	}
	for(int i = (DIRECTIONS_MAP_SIZE * 2) - 1; i < (DIRECTIONS_MAP_SIZE * DIRECTIONS_MAP_SIZE) - 1; i+= DIRECTIONS_MAP_SIZE) {
		if (directionsMap[i]) {
			if (!lastVisitedTrue) {
				lastVisitedTrue = true;
				numDirections++;
			}
		} else {
			lastVisitedTrue = false;
		}
	}
	for(int i = (DIRECTIONS_MAP_SIZE * DIRECTIONS_MAP_SIZE) - 1; i >= (DIRECTIONS_MAP_SIZE * (DIRECTIONS_MAP_SIZE - 1)); i--) {
		if (directionsMap[i]) {
			if (!lastVisitedTrue) {
				lastVisitedTrue = true;
				numDirections++;
			}
		} else {
			lastVisitedTrue = false;
		}
	}
	for(int i = (DIRECTIONS_MAP_SIZE * (DIRECTIONS_MAP_SIZE - 2)); i > 0; i-= DIRECTIONS_MAP_SIZE) {
		if (directionsMap[i]) {
			if (!lastVisitedTrue) {
				lastVisitedTrue = true;
				numDirections++;
			}
		} else {
			lastVisitedTrue = false;
		}
	}
	pixelMap[(row * PIXEL_MAP_WIDTH) + col] = numDirections;
}

unsigned int* getPixels() {
	unsigned int* pixelMap = new unsigned int[121]();
	for(int i = 0; i < 55; i++) {
		pixelMap[i] = 0;
	}
	for(int i = 55; i < 66; i++) {
		pixelMap[i] = 1;
	}
	for(int i = 66; i < 121; i++) {
		pixelMap[i] = 0;
	}
	pixelMap[5] = 1;
	pixelMap[16] = 1;
	pixelMap[27] = 1;
	pixelMap[38] = 1;
	pixelMap[49] = 1;
	pixelMap[60] = 1;
	pixelMap[71] = 1;
	pixelMap[82] = 1;
	pixelMap[93] = 1;
	pixelMap[104] = 1;
	pixelMap[115] = 1;
	// {
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	1,1,1,1,1,1,1,1,1,1,1,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// 	0,0,0,0,0,0,0,0,0,0,0,
	// });
	return pixelMap;
}