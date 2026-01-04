#include <iostream>
#include "MNISTLoader.h"
#include "GUI.h"

#define TRAINING false

using namespace std;

int main () {
	MNIST::Data training = MNIST::loadData("./dataset/train-images.idx3-ubyte","./dataset/train-labels.idx1-ubyte");
	
	Window w;
	
	w.requestShaderGen("MNIST","./assets/shaders/mnistVert.glsl","./assets/shaders/mnistFrag.glsl");

	
	Window::MNIST_Img img(w, (w.width - w.height + 10) / 2, 5, w.height - 10, w.height - 10);
	vector<uint8_t> data1 = training.getImage(4);
	cout << static_cast<int>(training.labels.at(4)) << endl;
	vector<GLfloat> data2(data1.size());
	for (int i = 0; i < data1.size(); i++) {
		data2[i] = static_cast<GLfloat>(data1[i]) / 255.0f;
	}
	img.inputMNISTData(data2);


	w.glThread->join();
	
	return 0;
}