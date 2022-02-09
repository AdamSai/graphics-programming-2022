#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>
#include <string>

// function declarations
// ---------------------
void createArrayBuffer(const std::vector<float>& array, unsigned int& VBO);

void setupShape(unsigned int shaderProgram, unsigned int& VAO, unsigned int& vertexCount, unsigned int& indicesCount);

void draw(unsigned int shaderProgram, unsigned int VAO, unsigned int vertexCount, const unsigned int indicesCount);


// glfw functions
// --------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);


// settings
// --------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


// shader programs
// ---------------
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 vtxColor; // output a color to the fragment shader\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   vtxColor = aColor;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in  vec3 vtxColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(vtxColor, 1.0);\n"
"}\n\0";


int main()
{

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif


	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	// build and compile our shader program
	// ------------------------------------

	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// link shaders
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	// setup vertex array object (VAO)
	// -------------------------------
	unsigned int VAO, vertexCount, indicesCount;
	// generate geometry in a vertex array object (VAO), record the number of vertices in the mesh,
	// tells the shader how to read it
	setupShape(shaderProgram, VAO, vertexCount, indicesCount);




	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(.2f, .2f, .2f, 1.0f); // background
		glClear(GL_COLOR_BUFFER_BIT); // clear the framebuffer

		draw(shaderProgram, VAO, vertexCount, indicesCount);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(
			window); // we normally use 2 frame buffers, a back (to draw on) and a front (to show on the screen)
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


// create a vertex buffer object (VBO) from an array of values, return VBO handle (set as reference)
// -------------------------------------------------------------------------------------------------
void createArrayBuffer(const std::vector<float>& array, unsigned int& VBO)
{
	// create the VBO on OpenGL and get a handle to it
	glGenBuffers(1, &VBO);
	// bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// set the content of the VBO (type, size, pointer to start, and how it is used)
	glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);
}


// create the geometry, a vertex array object representing it, and set how a shader program should read it
// -------------------------------------------------------------------------------------------------------
void setupShape(const unsigned int shaderProgram, unsigned int& VAO, unsigned int& vertexCount, unsigned int& indicesCount)
{

	float triangles = 180;
	float radius = 0.5f;
	auto attributes = std::vector<float>();
	auto indices = std::vector<unsigned int>();

	// Center
	attributes.push_back(0.0f);
	attributes.push_back(0.0f);
	attributes.push_back(0.0f);
	indices.push_back(0);

	int index = 1;
	for (int i = 0; i <= triangles; i++)
	{
		float x = radius * cos(i * 2 * M_PI / triangles);
		float y = radius * sin(i * 2 * M_PI / triangles);

		float x2 = radius * cos((i + 1) * 2 * M_PI / triangles);
		float y2 = radius * sin((i + 1) * 2 * M_PI / triangles);

		// TODO: Reuse previous vertex
		attributes.push_back(x);
		attributes.push_back(y);
		attributes.push_back(0.0f);

		indices.push_back(index++);

		attributes.push_back(x2);
		attributes.push_back(y2);
		attributes.push_back(0.0f);

		indices.push_back(index++);
		indices.push_back(0);

		// colors
		attributes.push_back(0.0f);
		attributes.push_back(1.0f);
		attributes.push_back(1.0f);

		attributes.push_back(1.0f);
		attributes.push_back(0.5f);
		attributes.push_back(0.5f);

		attributes.push_back(1.0f);
		attributes.push_back(0.5f);
		attributes.push_back(0.5f);
		// Indices are only used for vertex positions, so skip the color values
		index += 3;

	}

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);


	unsigned int VBO;
	createArrayBuffer(attributes, VBO);


	// tell how many vertices to draw
	vertexCount = attributes.size();
	indicesCount = indices.size();

	// create a vertex array object (VAO) on OpenGL and save a handle to it
	glGenVertexArrays(1, &VAO);

	// bind vertex array object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);


	//// set vertex shader attribute "aPos"
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);

	int posSize = 3;
	int posAttributeLocation = glGetAttribLocation(shaderProgram, "aPos");

	glEnableVertexAttribArray(posAttributeLocation);
	glVertexAttribPointer(posAttributeLocation, posSize, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, 0);

	// set vertex shader attribute "aColor"

	int colorSize = 3;
	int colorAttributeLocation = glGetAttribLocation(shaderProgram, "aColor");

	glEnableVertexAttribArray(colorAttributeLocation);
	glVertexAttribPointer(colorAttributeLocation, colorSize, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)(sizeof(GL_FLOAT) * 9));



}


// tell opengl to draw a vertex array object (VAO) using a give shaderProgram
// --------------------------------------------------------------------------
void draw(const unsigned int shaderProgram, const unsigned int VAO, const unsigned int vertexCount, const unsigned int indicesCount)
{
	// set active shader program
	glUseProgram(shaderProgram);
	// bind vertex array object
	glBindVertexArray(VAO);
	// draw geometry
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

