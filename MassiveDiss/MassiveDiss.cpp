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
	std::vector<ObjInfo>& objInfo
)
{
	for (int i = 0; i < objInfo.size(); i++)
	{
		if (objInfo[i].vertices.size() <= 0)
			continue;

		// Generate vertex buffer
		glGenBuffers(1, &objInfo[i].vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objInfo[i].vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].vertices.size() * sizeof(glm::vec3), &objInfo[i].vertices[0], GL_STATIC_DRAW);

		// Generate uv texture coordinate buffer
		glGenBuffers(1, &objInfo[i].uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objInfo[i].uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].uvs.size() * sizeof(glm::vec2), &objInfo[i].uvs[0], GL_STATIC_DRAW);

		// Generate normal buffer
		glGenBuffers(1, &objInfo[i].normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objInfo[i].normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].normals.size() * sizeof(glm::vec3), &objInfo[i].normals[0], GL_STATIC_DRAW);

		// Generate a buffer for the indices as well
		glGenBuffers(1, &objInfo[i].elembuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objInfo[i].elembuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objInfo[i].indices.size() * sizeof(unsigned int), &objInfo[i].indices[0], GL_STATIC_DRAW);
	}
}

void setupBoundingBox(
	std::vector<ObjInfo>& objInfo
)
{
	for (int i = 0; i < objInfo.size(); i++)
	{
		objInfo[i].boundingBox();

		glGenBuffers(1, &objInfo[i].positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objInfo[i].positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, objInfo[i].boundingVert.size() * sizeof(glm::vec3), &objInfo[i].boundingVert[0], GL_STATIC_DRAW);
	}
}

void cleanupBuffers(std::vector<ObjInfo> & objInfo)
{
	for (int i = 0; i < objInfo.size(); i++)
	{
		glDeleteBuffers(1, &objInfo[i].vertexbuffer);
		glDeleteBuffers(1, &objInfo[i].uvbuffer);
		glDeleteBuffers(1, &objInfo[i].normalbuffer);
		glDeleteBuffers(1, &objInfo[i].elembuffer);
	}
}

float depthCompare(ObjInfo a, ObjInfo b, glm::vec3 loc)
{
	glm::vec3 posA = loc - a.center();
	glm::vec3 posB = loc - b.center();


	return (posB.x * posB.x + posB.z * posB.z) - 
		(posA.x * posA.x + posA.z * posA.z);
}

std::vector<ObjInfo> mergeSortOrder(std::vector<ObjInfo> objInfo)
{
	if (objInfo.size() == 1)
		return objInfo;

	int half = (int)objInfo.size() / 2;

	std::vector<ObjInfo> firstHalf, secondHalf;
	for (int i = 0; i < objInfo.size(); i++)
	{
		if (i < half)
			firstHalf.push_back(objInfo[i]);
		else
			secondHalf.push_back(objInfo[i]);
	}

	firstHalf = mergeSortOrder(firstHalf);
	secondHalf = mergeSortOrder(secondHalf);

	std::vector<ObjInfo> res;

	bool stop = false;
	int i = 0, j = 0;
	while (!stop)
	{
		if (i < firstHalf.size() && j < secondHalf.size())
		{
			if (depthCompare(firstHalf[i], secondHalf[j], getPosition()) > 0.0f)
			{
				res.push_back(firstHalf[i]);
				i++;
			}
			else
			{
				res.push_back(secondHalf[j]);
				j++;
			}
		}
		else if (i >= firstHalf.size())
		{
			res.push_back(secondHalf[j]);
			j++;
		}
		else if (j >= secondHalf.size())
		{
			res.push_back(firstHalf[i]);
			i++;
		}

		if (i >= firstHalf.size() && j >= secondHalf.size())
			stop = true;
	}

	return res;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
	GLuint boundingBoxProgramID = LoadShaders("shaders/boundingbox.vertexshader", "shaders/boundingbox.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get handle for our "MVP" boundingbox uniform
	GLuint boundingMatrixID = glGetUniformLocation(boundingBoxProgramID, "MVP");

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

	std::vector<GLuint> textures;
	printf("Loading textures...\n");
	loadTextures(objInfo, textureLib, textures);

	std::vector<ObjInfo> indexed_objInfo;

	printf("Indexing VBOs...\n");
	indexVBO(objInfo, indexed_objInfo);

	printf("Ordering objects to be drawn...\n");
	indexed_objInfo = mergeSortOrder(indexed_objInfo);

	// Setting up buffers
	printf("Setting up buffers...\n");
	setupBuffers(indexed_objInfo);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// Initialize our little text library with the Holstein font
	printf("Initializing 2D textures...\n");
	initText2D("textures/Holstein.DDS");

	Frustum frustum;

	GLuint queryIDs[5];
	// Generate occlusion query names
	glGenQueries(5, queryIDs);

	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		GLenum mode = getMode() ? GL_FILL : GL_LINE;
		glPolygonMode(GL_FRONT_AND_BACK, mode);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(window, *monitor);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		frustum.update(ProjectionMatrix * ViewMatrix);

		if (getOccCheck())
		{
			indexed_objInfo = mergeSortOrder(indexed_objInfo);
		}

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		int bufferPadding = 0;
		int drawCalls = 0;
		int occludedObjects = 0;

		glm::vec3 lightPos = glm::vec3(4, 20, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		if (getOccCheck())
		{
			//glDisable(GL_TEXTURE_2D);
			//glShadeModel(GL_FLAT);

			//if (showBounding())
			//{
			//	glEnable(GL_POLYGON_STIPPLE);
			//}
			//else
			//{
			//	glDisable(GL_LIGHTING);
			//	glDisable(GL_COLOR_MATERIAL);
			//	glDisable(GL_NORMALIZE);
			//	glDepthMask(GL_FALSE);
			//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			//}

			//for (int i = 0; i < indexed_objInfo.size(); i++)
			//{
			//	// Use our shader
			//	glUseProgram(boundingBoxProgramID);

			//	//glBindVertexArray(indexed_objInfo[i].boundingBoxArray);
			//	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			//	// Check query results here (will be from previous frame or earlier)
			//	GLuint available, passed;
			//	glGetQueryObjectuiv(queryIDs[i], GL_QUERY_RESULT_AVAILABLE, &available);
			//	if (indexed_objInfo[i].queryInProgress && available) 
			//	{
			//		glGetQueryObjectuiv(queryIDs[i], GL_QUERY_RESULT, &passed);
			//		indexed_objInfo[i].occluded = passed == 0 ? true : false;
			//		if (indexed_objInfo[i].occluded) {
			//			occludedObjects++;
			//		}
			//		indexed_objInfo[i].queryInProgress = false;
			//	}


			//	if (!indexed_objInfo[i].queryInProgress)
			//	{
			//		glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, queryIDs[i]);
			//		glDrawArrays(GL_TRIANGLES, 0, indexed_objInfo[i].boundingBoxNumVertices);
			//		glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
			//		indexed_objInfo[i].queryInProgress = true;
			//	}
			//}
			//// Restore normal drawing state
			//glDisable(GL_POLYGON_STIPPLE);
			//glShadeModel(GL_SMOOTH);
			//glEnable(GL_LIGHTING);
			//glEnable(GL_COLOR_MATERIAL);
			//glEnable(GL_NORMALIZE);
			//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			//glDepthMask(GL_TRUE);
		}

		for (int i = 0; i < indexed_objInfo.size(); i++)
		{
			if (indexed_objInfo[i].vertices.size() < 1)
			{
				bufferPadding++;
				continue;
			}

			if (getVisCheck() && !frustum.checkSphere(indexed_objInfo[i].center(), indexed_objInfo[i].size()))
				continue;

			if (indexed_objInfo[i].occluded && getOccCheck())
				continue;

			// Use our shader
			glUseProgram(programID);

			// Turn on texturing
			glEnable(GL_TEXTURE_2D);

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
			glBindBuffer(GL_ARRAY_BUFFER, indexed_objInfo[i].vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : colors
			glBindBuffer(GL_ARRAY_BUFFER, indexed_objInfo[i].uvbuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 2, but must match the layout in the shader.
				2,                                // size : U+V => 2
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// 3rd attribute buffer : normals
			glBindBuffer(GL_ARRAY_BUFFER, indexed_objInfo[i].normalbuffer);
			glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexed_objInfo[i].elembuffer);

			// Draw the triangles !
			drawCalls++;
			glDrawElements(
				GL_TRIANGLES,				// mode
				(GLsizei)indexed_objInfo[i].indices.size(),    // count
				GL_UNSIGNED_INT,			// type
				(void*)0					// element array buffer offset
			);
		}

		glDeleteQueries(5, queryIDs);

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
		sprintf(text, "%d Occlusion Culling", getOccCheck());
		printText2D(text, 10, g_height - 277, 9 * g_width / g_height);
		sprintf(text, "%d Obj Occluded", occludedObjects);
		printText2D(text, 10, g_height - 298, 9 * g_width / g_height);
		sprintf(text, "%s", getMode() ? "GL_FILL" : "GL_LINE");
		printText2D(text, 10, g_height - 319, 9 * g_width / g_height);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup VBO and shader
	cleanupBuffers(indexed_objInfo);
	glDeleteProgram(programID);
	cleanupTextures(textures);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Clean up bounding box
	for (int i = 0; i < indexed_objInfo.size(); i++)
	{
		glDeleteBuffers(1, &indexed_objInfo[i].positionBuffer);
	}
	glDeleteProgram(boundingBoxProgramID);

	// Delete the text's VBO, the shader and the texture
	cleanupText2D();

	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

