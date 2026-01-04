#define GLEW_EXPERIMENTAL
#include "GUI.h"
#include <iostream>

using namespace std;

Window::Window () : Window(800,600) {};
Window::Window (int w, int h) : width(w), height(h), running(false) {
	glThread = new std::thread([this]() { this->init(); });
	// init();
}
void Window::init () {
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW" << endl;
		return;
	}

	window = glfwCreateWindow(width,height, "Number Recognition GUI", nullptr, nullptr);
	if (!window) {
		cerr << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
		return;
	}


	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);
	// glFrontFace(GL_CW);

	running.store(true);
	while(!glfwWindowShouldClose(window) && running.load()) {
		processInput();

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		for (auto s : shaderGenQueue) {
			shaders[s.name] = new Shader(s.vert,s.frag);
			shaders[s.name]->compile();
		}
		shaderGenQueue.clear();

		for (auto f : tickQueue) {
			f();
		}
		tickQueue.clear();

		for(auto o : objects) {
			if (o->flagReady) o->updateBuffers();
			if (o->rendering.load() && o->ready) o->draw(width, height);
		}


		glfwSwapBuffers(window);
	}


	running.store(false);
	
	//clear shaders
	for (auto [name, s] : shaders) {
		glDeleteProgram(s->ID);
	}
	//clear objects
	for (auto o : objects) {
		delete o;
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return;
}
void Window::processInput () {
	glfwPollEvents();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE)) running.store(false);
}

void Window::requestShaderGen (string name, const char* vertPath, const char* fragPath) {
	ShaderQueueObj o;
	o.name = name;
	o.vert = vertPath;
	o.frag = fragPath;
	shaderGenQueue.push_back(o);
}


Window::Object::Object (Window &window) : rendering(true), flagReady(false), ready(false), draw([](int w, int h){}) {
	window.objects.push_back(this);
	this->window = &window;
}
void Window::Object::cleanBuffers () {
	if (!ready) return;
	ready = false;
	if (glIsVertexArray(VAO)) glDeleteVertexArrays(1, &VAO);
	if (glIsBuffer(VBO)) glDeleteBuffers(1, &VBO);
	if (glIsBuffer(EBO)) glDeleteBuffers(1, &EBO);
}
void Window::Object::createBuffers () {
	if (!flagReady) return;
	cleanBuffers();

	float* v = vertices.data();
	unsigned int* i = indices.data();
	unsigned int* a = attr.data();

	totalIndices = indices.size();

	// Calculate attribute layout
	attrPerVert = 0; //Stride per Vertex
	unsigned int sums[attr.size()]; //offset per attribute
	for (int j = 0; j < attr.size(); j++) {
		sums[j] = attrPerVert;
		attrPerVert += a[j];
	}

	totalVertices = vertices.size() / attrPerVert;

	// Generate OpenGL buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), v, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), i, GL_STATIC_DRAW);

	for (int j = 0; j < attr.size(); j++) {
		glVertexAttribPointer(j, a[j], GL_FLOAT, GL_FALSE, attrPerVert * sizeof(float), (void*)(sums[j] * sizeof(float)));
		glEnableVertexAttribArray(j);
	}

	glBindVertexArray(0);
	ready = true;
	flagReady = false;
}
void Window::Object::updateBuffers () {
	if (!flagReady) return;
	if (!(vertices.size() == oldVert.size() && indices.size() == oldInd.size() && attr.size() == oldAttr.size()) || !(glIsVertexArray(VAO) && glIsBuffer(VBO) && glIsBuffer(EBO))) createBuffers();

	float* v = vertices.data();

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	oldVert = vertices;

	flagReady = false;
}

Window::MNIST_Img::MNIST_Img (Window &window, int x, int y, int w, int h) : obj(window) {
	this->window = &window;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

	r = 28;
	c = 28;
	
	obj.rendering = false;
	obj.attr = {2};
	obj.vertices = {
		0,0,
		1,0,
		1,1,
		0,1
	};
	obj.indices = {0,1,2,0,2,3};
	obj.draw = [&](int scrnW,int scrnH) {
		GLuint s = window.shaders["MNIST"]->ID;
		glUseProgram(s);

		glUniform2f(glGetUniformLocation(s, "screenDim"), scrnW,scrnH);
		glUniform2f(glGetUniformLocation(s, "pos"), this->x,this->y);
		glUniform2f(glGetUniformLocation(s, "dim"), this->w,this->h);
		glUniform2f(glGetUniformLocation(s, "grid"), this->r,this->c);
		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

		glBindVertexArray(obj.VAO);
		glDrawElements(GL_TRIANGLES, obj.totalIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	};
	obj.flagReady = true;
}
void Window::MNIST_Img::updateSSBO () {
	window->tickQueue.push_back([&]() {
		vector<GLfloat> arr(r*c);
		for (int x = 0; x < data.size(); x++) {
			for (int y = 0; y < data.at(x).size(); y++) {
				arr[x*c + y] = data.at(x).at(y);
			}
		}
		if (ssbo == 0) {
			glGenBuffers(1, &ssbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * r * c, arr.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		} else {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * r * c, 0, arr.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		obj.rendering = true;
	});
}
void Window::MNIST_Img::inputMNISTData (vector<GLfloat> arr) {
	data.clear();
	if (arr.size() != r * c) return;
	data.resize(r);
	for (int x = 0; x < r; x++) {
		data.at(x).resize(c);
		for (int y = 0; y < c; y++) {
			data[x][y] = arr[x*c + y];
		}
	}
	updateSSBO();
}