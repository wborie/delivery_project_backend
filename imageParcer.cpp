#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

int HEADER_SIZE = 54;

unsigned int* getPixels();
void findPaths(unsigned int* pixelMap, int row, int col, int initialRow, int initialCol, bool* visited, 
	unsigned int* directionsMap);

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
	for(int row = 0; row < 11; row++) {
		for(int col = 0; col < 11; col++) {
			if (pixelMap[(row * 11) + col]) { // Pixel is true
				bool* visited = new bool[121];
				for(int i = 0; i < 121; i++) {
					visited[i] = 0;
				}
				unsigned int* directionsMap = new unsigned int[121];
				for(int i = 0; i < 121; i++) {
					directionsMap[i] = 0;
				}
				findPaths(pixelMap, row, col, row, col, visited, directionsMap);
				for(int i = 0; i < 11; i++) {
					for(int j = 0; j < 11; j++) {
						std::cout << directionsMap[(i * 11) + j] << " ";
					}
					std::cout << std::endl;
				}
				std::cout << std::endl << "=============" << std::endl << std::endl;
				delete[] visited;
				delete[] directionsMap;
			}
		}
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

// Returns nothing, modifies the directionsMap
void findPaths(unsigned int* pixelMap, int row, int col, int initialRow, int initialCol, bool* visited, 
	unsigned int* directionsMap) {
	if (row < 0 || col < 0 || row >= 11 || col >= 11 || row < initialRow - 5 || row > initialRow + 5 || 
		col < initialCol - 5 || col > initialCol + 5) {
		return;
	} else if (visited[(row * 11) + col]) {
		return;
	// } else if (row == 0 && row != initialRow - 5) { // if too close to top, can't check 'up' direction
	// 	return;
	// } else if (col == 0 && col != initialCol - 5) { // if too close to left, can't check 'left' direction
	// 	return;
	// } else if (row == 11 && row != initialRow + 5) { // if too close to bottom, can't check 'down' direction
	// 	return;
	// } else if (col == 11 && col != initialCol + 5) { // if too close to right, can't check 'right' direction
	// 	return;
	} else if ((row == initialRow - 5 || row == initialRow + 5 || col == initialCol - 5 || col == initialCol + 5) && 
		pixelMap[(row * 11) + col]) {
		visited[(row * 11) + col] = 1;
		directionsMap[(row * 11) + col] = 1;
		return;
	} else if ((row == initialRow - 5 || row == initialRow + 5 || col == initialCol - 5 || col == initialCol + 5) && 
		!pixelMap[(row * 11) + col]) {
		// directionsMap[row][col] = 0;
		visited[(row * 11) + col] = 1;
		return;
	} else {
		visited[(row * 11) + col] = 1;
		if (row - 1 >= 0 && pixelMap[((row - 1) * 11) + col]) {
			findPaths(pixelMap, row - 1, col, initialRow, initialCol, visited, directionsMap);
		}
		if (row + 1 < 11 && pixelMap[((row + 1) * 11) + col]) {
			findPaths(pixelMap, row + 1, col, initialRow, initialCol, visited, directionsMap);
		}
		if (col - 1 >= 0 && pixelMap[(row * 11) + col - 1]) {
			findPaths(pixelMap, row, col - 1, initialRow, initialCol, visited, directionsMap);
		}
		if (col + 1 < 11 && pixelMap[(row * 11) + col + 1]) {
			findPaths(pixelMap, row, col + 1, initialRow, initialCol, visited, directionsMap);
		}
		return;
	}
}

unsigned int* getPixels() {
	unsigned int* pixelMap = new unsigned int[121]();
	for(int i = 0; i < 44; i++) {
		pixelMap[i] = 0;
	}
	for(int i = 44; i < 55; i++) {
		pixelMap[i] = 1;
	}
	for(int i = 55; i < 121; i++) {
		pixelMap[i] = 0;
	}
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