#include <iostream>

int main(int argc, const char* argv[]) {
	std::cout << argv[0] << " " << argc << std::endl;
	std::cout << "hello world!" << std::endl;
	if (argc == 3) {
		std::cout << argv[1] << " " << argv[2] << std::endl;
	}
}