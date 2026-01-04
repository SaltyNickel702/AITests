#ifndef MNIST_LOADER_H
#define MNIST_LOADER_H

#include <cstdint>
#include <vector>
#include <iostream>

namespace MNIST {
	struct Data {
		uint32_t count;
		uint32_t c;
		uint32_t r;
		std::vector<uint8_t> images;
		std::vector<uint8_t> labels;

		std::vector<uint8_t> getImage (unsigned int index);
	};

	Data loadData (const char* imagePath, const char* labelPath);
}


#endif