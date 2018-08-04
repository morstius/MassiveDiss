// MassiveDiss.cpp : Defines the entry point for the console application.
//

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <numeric>

#include "GL/glew.h"

// Include GLFW
#include "GLFW/glfw3.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/shader.h"
#include "common/texture.h"
#include "common/controls.h"
#include "common/text2D.h"

#include "ModelLoader.hpp"
#include "common/types/MtlObj.h"

#include "profile.h"
#include "kdTree.h"
#include "readWrite.h"
#include "common/frustum.h"

GLuint BoundingBoxProgramID;
GLuint BoundingVertBoxBufferID;
GLuint BoundingMatrixID;
GLuint ProgramID;
GLuint TextureID;

void error_callback(int error, const char* description)
{
	// a error callback for the glfw
	fprintf(stderr, "Error: %s\n", description);
}

void framecounting(int &numbFrames, double &time_last, double& time)
{
	// increase number of frames
	numbFrames++;

	// check how much time has passed
	if (glfwGetTime() - time_last >= 1.0)
	{
		// we want to know how much time was spent per frame
		time = 1000.0 / double(numbFrames);

		// need to zero the number of frames
		numbFrames = 0;
		// last time can now increase by one
		time_last += 1.0;
	}
}

void loadTextures(
	std::vector<ObjInfo>& objInfo,
	std::vector<MtlObj>& txtLib,
	std::vector<GLuint>& textures
)
{
	// adding a default texture for things that don't have a texture
	GLuint nr = loadDDS("textures/uvmap.DDS");
	textures.push_back(nr);

	for (int i = 0; i < objInfo.size(); i++)
	{
		int j = i - 1;
		bool check = true;
		while (j >= 0)
		{
			// checking if texture has been loaded
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
			// don't want to do anything in this specific case
		}
		else
		{
			// when we have no texture we default to the colourful uvmap.DDS
			objInfo[i].txIdx = -1;
		}
	}
}

// cleanup textures when closing
void cleanupTextures(std::vector<GLuint> textures)
{
	for (int i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i]);
	}
}

// setup buffers for objects
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

// setup bounding boxes per object and creating buffers for them
void createBoundingBox(
	std::vector<ObjInfo>& objInfo
)
{
	for (int i = 0; i < objInfo.size(); i++)
	{
		objInfo[i].createBoundingBox();
	}
}

// clean up buffers after closing
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

void generateQueries(std::vector<ObjInfo>& objInfo)
{
	for (int i = 0; i < objInfo.size(); ++i)
	{
		glGenQueries(1, &objInfo[i].queryID);

		objInfo[i].queryInProgress = false;
		objInfo[i].occluded = false;
	}
}

void cleanupQueries(std::vector<ObjInfo>& objInfo)
{
	for (int i = 0; i < objInfo.size(); ++i)
	{
		glDeleteQueries(1, &objInfo[i].queryID);
	}
}

bool depthCompare(const ObjInfo& a, const ObjInfo& b)
{
	glm::vec3 pos = getPosition();
	glm::vec3 posA = pos - a._center;
	glm::vec3 posB = pos - b._center;


	return (posB.x * posB.x + posB.z * posB.z) >
		(posA.x * posA.x + posA.z * posA.z);
}

void draw(
	ObjInfo& objInfo,
	Frustum& frustum,
	int& bufferPadding,
	int& occludedObjects,
	int& drawCalls,
	glm::mat4& MVP,
	std::vector<MtlObj>& textureLib,
	std::vector<GLuint>& textures
	)
{

	// if the object has no vertices then continue
	if (objInfo.vertices.size() < 1)
	{
		bufferPadding++;
		return;
	}

	// if using frustum culling and bounding sphere is not inside the frustum then it is not drawn
	if (getVisCheck() && !frustum.checkSphere(objInfo.center(), objInfo.size()))
		return;

	if (getOccCheck())
	{
		// used for proof of concept that something got occluded
		//std::sort(indexed_objInfo.begin(), indexed_objInfo.end(), depthCompare);

		glColorMask(false, false, false, false);
		glDepthMask(false);

		glUseProgram(BoundingBoxProgramID);

		glBindBuffer(GL_ARRAY_BUFFER, BoundingVertBoxBufferID);
		glBufferData(GL_ARRAY_BUFFER, objInfo.boundingVert.size() * sizeof(glm::vec3), &objInfo.boundingVert[0], GL_STATIC_DRAW);

		if (showBounding())
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniformMatrix4fv(BoundingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// query variables
		GLuint available;
		GLint passed;

		glGetQueryObjectuiv(objInfo.queryID, GL_QUERY_RESULT_AVAILABLE, &available);

		// try to get query results
		if (objInfo.queryInProgress && available)
		{
			glGetQueryObjectiv(objInfo.queryID, GL_QUERY_RESULT, &passed);
			objInfo.occluded = passed > 0 ? false : true;
			if (objInfo.occluded)
				occludedObjects++;

			objInfo.queryInProgress = false;
		}

		if (!objInfo.queryInProgress)
		{
			glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, objInfo.queryID);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, BoundingVertBoxBufferID);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, objInfo.boundingVert.size());

			glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);

			glDisableVertexAttribArray(0);

			objInfo.queryInProgress = true;
		}

		// turning the color and depth masks back on
		glColorMask(true, true, true, true);
		glDepthMask(true);
	}
	else
		objInfo.occluded = false;

	glUseProgram(ProgramID);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// if object is occluded then not drawing the object
	if (objInfo.occluded && getOccCheck())
		return;

	// switch to the texture that we want to use
	if (objInfo.txIdx >= 0 && textureLib[objInfo.txIdx].hasTexture)
	{
		// Bind our texture in Texture Unit 0
		// need to shift depending on the texture
		glActiveTexture(GL_TEXTURE0 + textureLib[objInfo.txIdx].textNbr);
		glBindTexture(GL_TEXTURE_2D, textures[textureLib[objInfo.txIdx].textNbr - 1]);
		// Set our "texSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, textureLib[objInfo.txIdx].textNbr);
	}
	else
	{
		// Bind our default texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		// Set our "texSampler" sampler to use the default Texture Unit 0
		glUniform1i(TextureID, 0);
	}

	// layout(location = 0) attribute buffer [vertices]
	glBindBuffer(GL_ARRAY_BUFFER, objInfo.vertexbuffer);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	// layout(location = 1) attribute buffer [colors]
	glBindBuffer(GL_ARRAY_BUFFER, objInfo.uvbuffer);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	// layout(location = 2) attribute buffer [normals]
	glBindBuffer(GL_ARRAY_BUFFER, objInfo.normalbuffer);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objInfo.elembuffer);

	glEnable(GL_DEPTH_TEST);
	// turn on blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw the triangles!
	drawCalls++;
	glDrawElements(
		GL_TRIANGLES,
		(GLsizei)objInfo.indices.size(),
		GL_UNSIGNED_INT,
		(void*)0
	);

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void frontBack(
	kdNode* tree,
	Frustum& frustum,
	int& bufferPadding,
	int& occludedObjects,
	int& drawCalls,
	glm::vec3& pos,
	glm::mat4& MVP,
	std::vector<MtlObj>& textureLib,
	std::vector<GLuint>& textures
)
{
	if (tree->isLeaf)
	{
		// reached at some objects to draw
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			draw(tree->objInfo[i], frustum, bufferPadding, occludedObjects, drawCalls,
				MVP, textureLib, textures);
		}
	}
	else
	{
		// ordering which brach to go down first
		if (tree->axis == kdNode::axisSplit::xaxis)
		{
			if (pos.x >= tree->split.x)
			{
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
			}
			else
			{
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
			}
		}
		if (tree->axis == kdNode::axisSplit::zaxis)
		{
			if (pos.z < tree->split.z)
			{
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
			}
			else
			{
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, drawCalls,
					pos, MVP, textureLib, textures);
			}
		}
	}
}

int main()
{
	// window object
	GLFWwindow* window;

	double frameCounter = 0.0;
	int g_width = 1024, g_height = 768;

	if (!glfwInit())
	{
		// failed to initialize GLFW so we need to print it out
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	// setting the error callback
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// open a window and create its OpenGL context
	window = glfwCreateWindow(g_width, g_height, "Project Window", NULL, NULL);

	// create monitor object for window resizing
	int* mon = new int[2];
	GLFWmonitor** monitor = glfwGetMonitors(mon);

	if (!window)
	{
		// window or OpenGL context creation failed
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	// initializing GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, g_width / 2, g_height / 2);

	// setting up error enum for probing
	GLenum err = glGetError();

	// dark blue background for contrast
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// cull backface
	glEnable(GL_CULL_FACE);

	// creating vertex arrays
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// create and compile our GLSL program from the shaders
	ProgramID = LoadShaders("shaders/base.vertexshader", "shaders/base.fragmentshader");
	// same for a bounding box shader
	BoundingBoxProgramID = LoadShaders("shaders/boundingbox.vertexshader", "shaders/boundingbox.fragmentshader");

	// get a handle for our uniform matrices
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(ProgramID, "View");
	GLuint ModelMatrixID = glGetUniformLocation(ProgramID, "Model");

	// need a handle for the bounding box one as well
	BoundingMatrixID = glGetUniformLocation(BoundingBoxProgramID, "MVP");

	// get a handle for our "texSampler" uniform
	TextureID = glGetUniformLocation(ProgramID, "texSampler");

	// setting up variables for framecounter
	double lastTime = glfwGetTime();
	int numFrames = 0;

	// creating a vector for the texture lib
	std::vector<MtlObj> textureLib;
	// creating a vector for the indexed objects
	std::vector<ObjInfo> indexed_objInfo;

	// selecting the model to use
	std::string model = "models/san-miguel";
	
	// if a .bin file exists then load that instead
	if (FILE *file = fopen(std::string(model + ".bin").c_str(), "r"))
	{
		fclose(file);

		printf("Reading in .bin file...\n");
		readBinaryFile(indexed_objInfo, model);
		readMtlFile(textureLib, model);
	}
	else
	{
		// otherwise load .obj file and index it and check for duplicates
		std::vector<ObjInfo> objInfo;
		std::vector<int> textInd;
		printf("Reading in .obj file...\n");
		bool res = loadObj(std::string(model + ".obj").c_str(), objInfo, textureLib);

		printf("Indexing VBOs...\n");
		vboIndex(objInfo, indexed_objInfo);
		objInfo.clear();

		// create bounding boxes
		printf("Creating bouding boxes...\n");
		createBoundingBox(indexed_objInfo);

		// write out binary
		printf("Write out .bin file...\n");
		writeBinaryFile(indexed_objInfo, model);
		writeMtlFile(textureLib, model);
	}

	// if profiling is needed
	/*printf("Profiling...\n");
	profileKdTree(indexed_objInfo);*/

	// load in teaxtures
	std::vector<GLuint> textures;
	printf("Loading textures...\n");
	loadTextures(indexed_objInfo, textureLib, textures);

	// set up the buffers, a buffer per object
	printf("Setting up buffers...\n");
	setupBuffers(indexed_objInfo);

	// generate occlusion query names
	printf("Generate queries...\n");
	generateQueries(indexed_objInfo);

	// create k-d tree
	printf("Creating k-d tree...\n");
	kdNode* tree = kdTreeConstruct(indexed_objInfo, 0, indexed_objInfo.size());

	// initialize the bounding box VBO
	glGenBuffers(1, &BoundingVertBoxBufferID);

	// get a handle for our "LightPos" uniform to have a light available
	// looks good in the courtyard of san-miguel
	glUseProgram(ProgramID);
	GLuint lightID = glGetUniformLocation(ProgramID, "LightPos");

	// setting the light position
	glm::vec3 lightPos = glm::vec3(14, 15, 6); // this positioning is picked with the san-miguel model in mind
	glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);

	// initialize the 2D text for stats
	printf("Initializing 2D textures...\n");
	initText2D("textures/font.png");

	// creating a frustum object
	Frustum frustum;

	// counters
	int bufferPadding = 0;
	int drawCalls = 0;
	int occludedObjects = 0;

	while (!glfwWindowShouldClose(window))
	{
		// clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use our shader
		glUseProgram(ProgramID);

		// posibility of switching from fill to line, so triangles are filled or just outlined
		GLenum mode = getMode() ? GL_FILL : GL_LINE;
		glPolygonMode(GL_FRONT_AND_BACK, mode);

		// compute the MVP matrix from keyboard and mouse input
		computeMVP(window, *monitor, g_width, g_height);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// updating the view frustum
		if(getVisCheck())
			frustum.update(ProjectionMatrix * ViewMatrix);

		// send transforms to shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		// counters
		bufferPadding = 0;
		drawCalls = 0;
		occludedObjects = 0;

		glm::vec3 pos = getPosition();

		// draw the objects in front to back order
		frontBack(tree, frustum, bufferPadding, occludedObjects, drawCalls,
			pos, MVP, textureLib, textures);

		// update frame counter and print on screen
		framecounting(numFrames, lastTime, frameCounter);

		// draw stats
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

	// delete queries
	cleanupQueries(indexed_objInfo);

	// cleanup VBO and shader
	cleanupBuffers(indexed_objInfo);
	glDeleteProgram(ProgramID);
	cleanupTextures(textures);
	glDeleteVertexArrays(1, &VertexArrayID);

	glDeleteProgram(BoundingBoxProgramID);

	// delete the text's VBO, the shader and the texture
	cleanupText2D();

	// close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

