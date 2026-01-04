#include "MNISTLoader.h"
#include <fstream>

using namespace std;


uint32_t read4Bytes(ifstream &f) { //reads 4 bytes (hence 32 bit int) of the specified file
    uint8_t bytes[4];
    f.read(reinterpret_cast<char*>(bytes), 4);
    return (uint32_t(bytes[0]) << 24) |
           (uint32_t(bytes[1]) << 16) |
           (uint32_t(bytes[2]) << 8)  |
           (uint32_t(bytes[3]));
}

MNIST::Data MNIST::loadData (const char* imagePath, const char* labelPath) {
	ifstream images(imagePath, ios::binary);
	if (!images) throw runtime_error("Failed to open images file: " + string(imagePath));
	ifstream labels(labelPath, ios::binary);
	if (!labels) throw runtime_error("Failed to open labels file: " + string(labelPath));

	uint32_t idImages = read4Bytes(images);
	if (idImages != 2051) throw runtime_error("Invalid image file"); // MNIST Image files are identified by 2051 in the first 4 bytes
	uint32_t idLabels = read4Bytes(labels);
	if (idLabels != 2049) throw runtime_error("Invalid label file"); // MNIST Label files are identified by 2049 in the first 4 bytes


	//Image data first
	Data data;
	data.count = read4Bytes(images);
	data.r = read4Bytes(images);
	data.c = read4Bytes(images);

	data.images.resize(data.count * data.c * data.r);
	images.read(reinterpret_cast<char*>(data.images.data()), data.images.size());

	uint32_t lblCount = read4Bytes(labels); //should be same as image count
	data.labels.resize(lblCount);
	labels.read(reinterpret_cast<char*>(data.labels.data()), data.labels.size());


	return data;
}
vector<uint8_t> MNIST::Data::getImage (unsigned int index) {
	if (index >= count) runtime_error("Invalid index");
	vector<uint8_t> v;
	for (int i = index * r * c; i < (index + 1) * r * c; i++) {
		v.push_back(images.at(i));
	}
	return v;
}