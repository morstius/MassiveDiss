// MassiveDiss.cpp : Defines the entry point for the console application.
//

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "GL/glew.h"

// Include GLFW
#include "GLFW/glfw3.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "common/vboindexer.hpp"
#include "common/text2D.hpp"

#include "ModelLoader.hpp"
#include "common/types/MtlObj.h"
#include "common/types/Buffers.h"

#include "kdTree.h"
#include "common/frustum.h"
#include "common/helpers/xline.h"

#define COMBINE_SAME_TEXTURES false

// Some bad globals
double frameCounter = 0.0;
int g_width = 1024, g_height = 768;

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void framecounting(int &nbFrames, double &lastTime)
{
	// Measure speed
	double currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
										 // printf and reset timer
		frameCounter = 1000.0 / double(nbFrames);
		//printf("%f ms/frame\n", frameCounter);
		nbFrames = 0;
		lastTime += 1.0;
	}
}

void objIndex(std::vector<ObjInfo> objInfo, std::vector<int>& objIdx)
{
	for (int i = 0; i < objInfo.size(); i++)
	{
		if (objInfo[i].indices.size() > 0)
			objIdx.push_back(i);
	}
}

void populateTreeIdx(kdNode* tree, std::vector<ObjInfo> objInfo)
{
	if (tree != nullptr)
	{
		for (int i = 0; i < objInfo.size(); i++)
		{
			if (tree->objinfo.center() == objInfo[i].center() && tree->objinfo.vertices.size() == objInfo[i].vertices.size())
				tree->idx = i;
		}

		populateTreeIdx(tree->left, objInfo);
		populateTreeIdx(tree->right, objInfo);
	}
}

void loadTextures(
	std::vector<ObjInfo>& objInfo,
	std::vector<MtlObj>& txtLib,
	std::vector<GLuint>& textures
)
{
	// adding a default white
	GLuint nr = loadDDS("textures/uvmap.DDS");
	textures.push_back(nr);

	for (int i = 0; i < objInfo.size(); i++)
	{
		int j = i - 1;
		bool check = true;
		while (j >= 0)
		{
			if (objInfo[i].txIdx == objInfo[j].txIdx)
				check = false;
			j--;
		}

		if (check && objInfo[i].txIdx >= 0 && txtLib[objInfo[i].txIdx].hasTexture)
		{
			GLuint tNumb = loadStbText(txtLib[objInfo[i].txIdx].map_Kd.c_str());
			txtLib[objInfo[i].txIdx].textNbr = tNumb;
			textures.push_back(tNumb);
		}
		else if (!check)
		{

		}
		else
		{
			// when we have no texture we default to the colourful uvmap.DDS
			objInfo[i].txIdx = -1;
		}
	}
}

void cleanupTextures(std::vector<GLuint> textures)
{
	// cleanup textures
	for (int i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i]);
	}
}

void setupBuffers(
	std::vector<ObjInfo>& objInfo,
	std::vector<Buffers> & buffers
)
{
	for (int i = 0; i < objInfo.size(); i++)
	{
		if (objInfo[i].vertices.size() <= 0)
			continue;

		Buffers buffer = Buffers();

		// Generate vertex buffer
		glGenBuffers(1, &buffer.vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer.vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].vertices.size() * sizeof(glm::vec3), &objInfo[i].vertices[0], GL_STATIC_DRAW);

		// Generate uv texture coordinate buffer
		glGenBuffers(1, &buffer.uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer.uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].uvs.size() * sizeof(glm::vec2), &objInfo[i].uvs[0], GL_STATIC_DRAW);

		// Generate normal buffer
		glGenBuffers(1, &buffer.normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer.normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].normals.size() * sizeof(glm::vec3), &objInfo[i].normals[0], GL_STATIC_DRAW);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &buffer.elembuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.elembuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objInfo[i].indices.size() * sizeof(unsigned int), &objInfo[i].indices[0], GL_STATIC_DRAW);

		buffers.push_back(buffer);
	}
}

void cleanupBuffers(std::vector<Buffers> & buffers)
{
	for (int i = 0; i < buffers.size(); i++)
	{
		glDeleteBuffers(1, &buffers[i].vertexbuffer);
		glDeleteBuffers(1, &buffers[i].uvbuffer);
		glDeleteBuffers(1, &buffers[i].normalbuffer);
		glDeleteBuffers(1, &buffers[i].elembuffer);
	}
}

int main()
{
	GLFWwindow* window;

	glewExperimental = true; // Needed for core profile
	if (!glfwInit())
	{
		// Initialization failed
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 4.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

																   // Open a window and create its OpenGL context
	window = glfwCreateWindow(g_width, g_height, "Project Window", NULL, NULL);

	int* mon = new int[2];
	GLFWmonitor** monitor = glfwGetMonitors(mon);

	if (!window)
	{
		// Window or OpenGL context creation failed
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, g_width / 2, g_height / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("shaders/base.vertexshader", "shaders/base.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Setting up variables for framecounter
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	std::vector<MtlObj> textureLib;

	// Read our .obj file
	std::vector<ObjInfo> objInfo;
	std::vector<int> textInd;
	bool res = loadObj("models/san-miguel.obj", objInfo, textureLib);

	//printf("Creating kd-Tree...\n");
	//kdNode* tree = kdTreeConstruct(objInfo, 0, (int)objInfo.size());

	std::vector<GLuint> textures;
	printf("Loading textures...\n");
	loadTextures(objInfo, textureLib, textures);

	std::vector<ObjInfo> indexed_objInfo;

	printf("Indexing VBOs...\n");
	indexVBO(objInfo, indexed_objInfo);

	//objIndex(indexed_objInfo, objIdx);
	//populateTreeIdx(tree, objInfo);

	// Setting up buffers
	printf("Setting up buffers...\n");
	std::vector<Buffers> buffers;
	setupBuffers(indexed_objInfo, buffers);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// Initialize our little text library with the Holstein font
	printf("Initializing 2D textures...\n");
	initText2D("textures/Holstein.DDS");

	Frustum frustum;
	std::vector<int> visibleSet;

	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(window, *monitor);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		frustum.update(ProjectionMatrix * ViewMatrix);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		int bufferPadding = 0;
		int drawCalls = 0;

		if (getVisCheck())
			visibleSet.clear();

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		for (int i = 0; i < indexed_objInfo.size(); i++)
		{
			if (indexed_objInfo[i].vertices.size() < 1)
			{
				bufferPadding++;
				continue;
			}

			if (getVisCheck() && !frustum.checkSphere(indexed_objInfo[i].center(), indexed_objInfo[i].size()))
				continue;

			if (!getVisCheck())
			{
				// keep drawing the set that was visible in the last frustum check
				if (std::find(visibleSet.begin(), visibleSet.end(), i) == visibleSet.end())
					continue;
			}

			visibleSet.push_back(i);

			if (indexed_objInfo[i].txIdx >= 0 && textureLib[indexed_objInfo[i].txIdx].hasTexture)
			{
				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0 + textureLib[indexed_objInfo[i].txIdx].textNbr);
				glBindTexture(GL_TEXTURE_2D, textures[textureLib[indexed_objInfo[i].txIdx].textNbr - 1]);
				// Set our "myTextureSampler" sampler to use Texture Unit 0
				glUniform1i(TextureID, textureLib[indexed_objInfo[i].txIdx].textNbr);
			}
			else
			{
				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[0]);
				// Set our "myTextureSampler" sampler to use Texture Unit 0
				glUniform1i(TextureID, 0);
			}

			// 1rst attribute buffer : vertices
			glBindBuffer(GL_ARRAY_BUFFER, buffers[i - bufferPadding].vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : colors
			glBindBuffer(GL_ARRAY_BUFFER, buffers[i - bufferPadding].uvbuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 2, but must match the layout in the shader.
				2,                                // size : U+V => 2
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// 3rd attribute buffer : normals
			glBindBuffer(GL_ARRAY_BUFFER, buffers[i - bufferPadding].normalbuffer);
			glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[i - bufferPadding].elembuffer);

			// Draw the triangles !
			drawCalls++;
			glDrawElements(
				GL_TRIANGLES,				// mode
				(GLsizei)indexed_objInfo[i].indices.size(),    // count
				GL_UNSIGNED_INT,			// type
				(void*)0					// element array buffer offset
			);
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);


		// update frame counter and print on screen
		framecounting(nbFrames, lastTime);

		char text[256];
		sprintf(text, "%d draw calls", drawCalls);
		printText2D(text, 10, g_height - 193, 9 * g_width / g_height);
		sprintf(text, "%.2f ms/frame", frameCounter);
		printText2D(text, 10, g_height - 214, 9 * g_width / g_height);
		sprintf(text, "%.2f fps", 1000.0 / frameCounter);
		printText2D(text, 10, g_height - 235, 9 * g_width / g_height);
		sprintf(text, "%d Frustum Culling", getVisCheck());
		printText2D(text, 10, g_height - 256, 9 * g_width / g_height);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup VBO and shader
	cleanupBuffers(buffers);
	glDeleteProgram(programID);
	cleanupTextures(textures);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Delete the text's VBO, the shader and the texture
	cleanupText2D();

	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

