#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include <thread>
#include <atomic>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>

struct Window {
	GLFWwindow* window;
	int width,height;
	std::thread* glThread;

	Window (int w, int h);
	Window ();

	std::atomic_bool running;
	struct Shader {
		Shader (std::string vertexPath,  std::string fragmentPath) : vP(vertexPath), fP(fragmentPath) {}
		std::string vP, fP;
		GLuint ID;
		bool readyToRun = false;

		void compile () {
			using namespace std;

			string vertexCode;
			string fragmentCode;

			ifstream vShader;
			ifstream fShader;

			vShader.exceptions(ifstream::failbit | ifstream::badbit);
			fShader.exceptions(ifstream::failbit | ifstream::badbit);

			//Read Files
			try {
				vShader.open(vP);
				fShader.open(fP);

				stringstream vShaderStream, fShaderStream;
				vShaderStream << vShader.rdbuf();
				fShaderStream << fShader.rdbuf();

				vShader.close();
				fShader.close();

				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			} catch (ifstream::failure e) {
				cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" <<endl;
			}
			const char* vertexCString = vertexCode.c_str();
			const char* fragmentCString = fragmentCode.c_str();


			//Compile Shaders
			unsigned int vertex, fragment;
			int success;
			char infoLog[512];

			//Vertex
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex,1,&vertexCString,NULL);
			glCompileShader(vertex);

			glGetShaderiv(vertex,GL_COMPILE_STATUS,&success);
			if (!success) {
				glGetShaderInfoLog(vertex,512,NULL,infoLog);
				cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
			}

			//fragment
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment,1,&fragmentCString,NULL);
			glCompileShader(fragment);

			glGetShaderiv(fragment,GL_COMPILE_STATUS,&success);
			if (!success) {
				glGetShaderInfoLog(fragment,512,NULL,infoLog);
				cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
			}

			
			//Program Creation
			ID = glCreateProgram();
			glAttachShader(ID,vertex);
			glAttachShader(ID,fragment);
			glLinkProgram(ID);

			glGetProgramiv(ID,GL_LINK_STATUS,&success);
			if (!success) {
				glGetShaderInfoLog(fragment,512,NULL,infoLog);
				cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
			}

			glDeleteShader(vertex);
			glDeleteShader(fragment);

			readyToRun = true;
		}
		~Shader () {
			if (ID != 0) glDeleteProgram(ID);
		}
	};
	std::map<std::string,Shader*> shaders;
	void requestShaderGen (std::string name, const char* vertPath, const char* fragPath);
	
	struct Object {
		Object () = delete;
		Object (Window &window);

		std::atomic_bool rendering; // Sets if the item is being rendered

		Window* window;

		std::atomic_bool flagReady; //Ready to be pushed to buffer
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		std::vector<unsigned int> attr;

		bool ready; //Ready to be drawn
		unsigned int VAO, EBO, VBO;

		unsigned int totalIndices;
		unsigned int totalVertices;
		unsigned int attrPerVert;

		void createBuffers (); //must be called on render thread
		void updateBuffers (); //If data size is the same
		void cleanBuffers ();

		std::function<void(int scrnW, int scrnH)> draw;

		~Object () {
			cleanBuffers();
			
			//clear from objects vector
			auto pointer = std::find(window->objects.begin(), window->objects.end(), this);
			if (pointer != window->objects.end()) {
				window->objects.erase(pointer);
			}
		}

		private:
			std::vector<float> oldVert;
			std::vector<float> oldInd;
			std::vector<float> oldAttr;
	};
	std::vector<Object*> objects;


	std::vector<std::function<void()>> tickQueue;

	struct MNIST_Img {
		MNIST_Img () = delete;
		MNIST_Img (Window &window) : MNIST_Img(window,0,0,100,100) {};
		MNIST_Img (Window &window, int x, int y, int w, int h);
		Object obj;

		Window* window;

		int r,c;
		std::vector<std::vector<GLfloat>> data;
		GLuint ssbo;
		void updateSSBO ();
		void inputMNISTData (std::vector<GLfloat> arr);

		int x,y;
		int w,h;
	};

	struct ShaderQueueObj {
		std::string name;
		const char* vert;
		const char* frag;
	};
	std::vector<ShaderQueueObj> shaderGenQueue;
	private:
		void init ();
		void processInput ();
};

#endif