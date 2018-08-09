// MassiveDiss.cpp : Defines the entry point for the console application.
//

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <numeric>
#include <map>

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
#include "common/occluder.h"

#include "ModelLoader.hpp"
#include "common/types/MtlObj.h"

#include "profile.h"
#include "kdTree.h"
#include "readWrite.h"
#include "common/frustum.h"
#include "common/vr.h"

#define USE_VR true

GLuint BoundingBoxProgramID;
GLuint BoundingVertBoxBufferID;
GLuint BoundingMatrixID;
GLuint ProgramID;
GLuint TextureID;
GLuint VRID;
GLuint m_unSceneVAO;
GLuint m_nSceneMatrixLocation;

FramebufferDesc leftEyeDesc;
FramebufferDesc rightEyeDesc;
int vrWidth, vrHeight;

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
		time = 1000.0 / (double)numbFrames;

		// reset number of frames to 0
		numbFrames = 0;
		// last time can now increase by one
		time_last += 1.0;
	}
}

void loadTextures(
	kdNode* tree,
	std::vector<MtlObj>& txtLib,
	std::vector<GLuint>& textures,
	std::map<int, int>& map
)
{
	if (tree->isLeaf)
	{
		for (int i = 0; i < tree->objInfo.size(); i++)
		{
			// check if texture has already been loaded
			bool check = true;
			if (map.find(tree->objInfo[i].txIdx) != map.end())
				check = false;

			if (check && tree->objInfo[i].txIdx >= 0 && txtLib[tree->objInfo[i].txIdx].hasTexture)
			{
				// load texture and add it to the vector of textures to use
				GLuint tNumb = loadStbText(txtLib[tree->objInfo[i].txIdx].map_Kd.c_str());
				txtLib[tree->objInfo[i].txIdx].textNbr = tNumb;
				textures.push_back(tNumb);
				map[tree->objInfo[i].txIdx] = tree->objInfo[i].txIdx;
			}
			else if (!check)
			{
				// don't want to do anything in this specific case
			}
			else
			{
				// when we have no texture we default to the colourful uvmap.DDS
				tree->objInfo[i].txIdx = -1;
			}
		}
	}
	else
	{
		loadTextures(tree->left, txtLib, textures, map);
		loadTextures(tree->right, txtLib, textures, map);
	}
}

void loadTreeTextures(
	kdNode* tree,
	std::vector<MtlObj>& txtLib,
	std::vector<GLuint>& textures
)
{
	// adding a default texture for things that don't have a texture
	GLuint nr = loadDDS("textures/uvmap.DDS");
	textures.push_back(nr);

	std::map<int, int> map;
	loadTextures(tree, txtLib, textures, map);
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
	kdNode* tree
)
{
	if (tree->isLeaf)
	{
		// loop through all the objects
		for (int i = 0; i < tree->objInfo.size(); i++)
		{
			if (tree->objInfo[i].vertices.size() <= 0)
				continue;

			// Generate vertex buffer
			glGenBuffers(1, &tree->objInfo[i].vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, tree->objInfo[i].vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, tree->objInfo[i].vertices.size() * sizeof(glm::vec3), &tree->objInfo[i].vertices[0], GL_STATIC_DRAW);

			// Generate uv texture coordinate buffer
			glGenBuffers(1, &tree->objInfo[i].uvbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, tree->objInfo[i].uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, tree->objInfo[i].uvs.size() * sizeof(glm::vec2), &tree->objInfo[i].uvs[0], GL_STATIC_DRAW);

			// Generate normal buffer
			glGenBuffers(1, &tree->objInfo[i].normalbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, tree->objInfo[i].normalbuffer);
			glBufferData(GL_ARRAY_BUFFER, tree->objInfo[i].normals.size() * sizeof(glm::vec3), &tree->objInfo[i].normals[0], GL_STATIC_DRAW);

			// Generate a buffer for the indices as well
			glGenBuffers(1, &tree->objInfo[i].elembuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tree->objInfo[i].elembuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, tree->objInfo[i].indices.size() * sizeof(unsigned int), &tree->objInfo[i].indices[0], GL_STATIC_DRAW);
		}
	}
	else
	{
		setupBuffers(tree->left);
		setupBuffers(tree->right);
	}
}

// setup bounding boxes per object and creating buffers for them
void createBoundingBox(
	std::vector<ObjInfo>& objInfo,
	std::vector<MtlObj>& mtlObj
)
{
	// loop through all the objects and create a bounding box for them
	for (int i = 0; i < objInfo.size(); i++)
	{
		objInfo[i].createBoundingBox();

		// checking if this object should be included in occlusion check
		if (isOccluder(mtlObj[objInfo[i].txIdx].newmtl))
		{
			objInfo[i].useForOcclusion = true;
		}
		else
			objInfo[i].useForOcclusion = false;
	}
}

// clean up buffers after closing
void cleanupBuffers(kdNode * tree)
{
	if (tree->isLeaf)
	{
		// cleanup the buffers this leaf
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			glDeleteBuffers(1, &tree->objInfo[i].vertexbuffer);
			glDeleteBuffers(1, &tree->objInfo[i].uvbuffer);
			glDeleteBuffers(1, &tree->objInfo[i].normalbuffer);
			glDeleteBuffers(1, &tree->objInfo[i].elembuffer);
		}
	}
	else
	{
		// recursively clean up left and right
		cleanupBuffers(tree->left);
		cleanupBuffers(tree->right);
	}
}

// generate queries for all the objects
void generateQueries(kdNode* tree)
{
	if (tree->isLeaf)
	{
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			glGenQueries(1, &tree->objInfo[i].queryID);

			tree->objInfo[i].queryInProgress = false;
			tree->objInfo[i].occluded = false;
		}
	}
	else
	{
		generateQueries(tree->left);
		generateQueries(tree->right);
	}
}

// cleanup the queries after window is closed
void cleanupQueries(kdNode* tree)
{
	if (tree->isLeaf)
	{
		// cleanup the querys this leaf
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			glDeleteQueries(1, &tree->objInfo[i].queryID);
		}
	}
	else
	{
		// recursively clean up left and right
		cleanupQueries(tree->left);
		cleanupQueries(tree->right);
	}
}

// the chunky draw command
void draw(
	ObjInfo& objInfo,
	Frustum& frustum,
	int& bufferPadding,
	int& occludedObjects,
	int& frustOccluded,
	int& drawCalls,
	glm::mat4& MVP,
	std::vector<MtlObj>& textureLib,
	std::vector<GLuint>& textures
	)
{

	// if using frustum culling and the bounding sphere is not inside the frustum then it is not drawn
	if (getVisCheck() && !frustum.checkSphere(objInfo.center(), objInfo.size()))
	{
		frustOccluded++;
		return;
	}

	// the occlusion check
	if (getOccCheck() && objInfo.useForOcclusion)
	{
		// turning off color and depth mask
		glColorMask(false, false, false, false);
		glDepthMask(false);

		// switch to the bounding box shaders
		glUseProgram(BoundingBoxProgramID);

		// binding the bounding box vertexbuffer
		glBindBuffer(GL_ARRAY_BUFFER, BoundingVertBoxBufferID);
		glBufferData(GL_ARRAY_BUFFER, objInfo.boundingVert.size() * sizeof(glm::vec3), &objInfo.boundingVert[0], GL_STATIC_DRAW);

		// need to update the MVP matrix
		glUniformMatrix4fv(BoundingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		glUniformMatrix4fv(m_nSceneMatrixLocation, 1, GL_FALSE, &MVP[0][0]);

		// query variables
		GLuint available;
		GLint passed;

		// check if there are results ready from a previous query
		glGetQueryObjectuiv(objInfo.queryID, GL_QUERY_RESULT_AVAILABLE, &available);

		// try to get query results
		if (objInfo.queryInProgress && available)
		{
			// checking if this queryID passed any fragments
			glGetQueryObjectiv(objInfo.queryID, GL_QUERY_RESULT, &passed);

			// if some passed then it is not occlude, otherwise it is occluded
			objInfo.occluded = passed > 0 ? false : true;

			// setting this flag to false so it will start a new query right away
			objInfo.queryInProgress = false;
		}

		// if a query is not in progress then starting a query
		if (!objInfo.queryInProgress)
		{
			// beginning the query
			glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, objInfo.queryID);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// drawing the bounding box for the object
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)objInfo.boundingVert.size());

			// ending the query
			glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);

			glDisableVertexAttribArray(0);

			// now wait until at least the next frame to check for results
			objInfo.queryInProgress = true;
		}

		// turning the color and depth masks back on
		glColorMask(true, true, true, true);
		glDepthMask(true);
	}

	// if object is occluded then not drawing the object
	if (getOccCheck())
	{
		if (!reverseOcclusionCulling() && objInfo.occluded)
		{
			occludedObjects++;
			return;
		}
	}

	// switch to the main drawing shader
	glUseProgram(ProgramID);

	if (USE_VR)
		glUseProgram(VRID);

	// posibility of switching from fill to line, so triangles are filled or just outlined
	GLenum mode = getMode() ? GL_FILL : GL_LINE;
	glPolygonMode(GL_FRONT_AND_BACK, mode);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// activating the default texture in Texture Unit 0
	glBindVertexArray(m_unSceneVAO);
	glActiveTexture(GL_TEXTURE0);

	// switch to the texture that we want to use
	if (objInfo.txIdx >= 0 && textureLib[objInfo.txIdx].hasTexture)
		glBindTexture(GL_TEXTURE_2D, textures[textureLib[objInfo.txIdx].textNbr - 1]);
	else
		glBindTexture(GL_TEXTURE_2D, textures[0]);

	// texture sampler using texture unit 0
	glUniform1i(TextureID, 0);

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

	// turn on blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// want to keep count of how many draw calls there are
	drawCalls++;

	// draw the triangles
	glDrawElements(
		GL_TRIANGLES,
		(GLsizei)objInfo.indices.size(),
		GL_UNSIGNED_INT,
		(void*)0
	);

	// disable blending
	glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindVertexArray(0);
	glUseProgram(0);
}

// traverse the k-d tree to draw in a front to back order
void frontBack(
	kdNode* tree,
	Frustum& frustum,
	int& bufferPadding,
	int& occludedObjects,
	int& frustOccluded,
	int& drawCalls,
	glm::vec3 pos,
	glm::mat4& MVP,
	std::vector<MtlObj>& textureLib,
	std::vector<GLuint>& textures
)
{
	if (tree->isLeaf)
	{
		// reached a leaf so I have some objects to draw
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			draw(tree->objInfo[i], frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls, MVP, textureLib, textures);
		}
	}
	else
	{
		// ordering which brach to go down first
		if (tree->axis == kdNode::axisSplit::xaxis)
		{
			if (pos.x < tree->split)
			{
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
			}
			else
			{
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
			}
		}
		if (tree->axis == kdNode::axisSplit::zaxis)
		{
			if (pos.z < tree->split)
			{
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
			}
			else
			{
				frontBack(tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
				frontBack(tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures);
			}
		}
	}
}

void RenderStereoTargets(
	kdNode* tree,
	Frustum& frustum,
	int& bufferPadding,
	int& occludedObjects,
	int& frustOccluded,
	int& drawCalls,
	glm::vec3 pos,
	glm::mat4& MVP,
	std::vector<MtlObj>& textureLib,
	std::vector<GLuint>& textures,
	VR vr)
{
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, vrWidth, vrHeight);
	glm::mat4 vrMVP = vr.GetCurrentViewProjectionMatrix(vr::Hmd_Eye::Eye_Left);
	frontBack(tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
		pos, vrMVP, textureLib, textures);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, vrWidth, vrHeight, 0, 0, vrWidth, vrHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);

	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, vrWidth, vrHeight);
	vrMVP = vr.GetCurrentViewProjectionMatrix(vr::Hmd_Eye::Eye_Right);
	frontBack(tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
		pos, vrMVP, textureLib, textures);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, vrWidth, vrHeight, 0, 0, vrWidth, vrHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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

	// some window and selecting openGL 4.4
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
		return -1;
	}

	glfwMakeContextCurrent(window);

	// turning off v-sync
	glfwSwapInterval(0);

	// initializing GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// hide the mouse and enable unlimited movement when the window is being selected
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// putting the mouse int the center of the window
	glfwPollEvents();
	glfwSetCursorPos(window, g_width / 2, g_height / 2);

	// setting up error enum for probing
	GLenum err = glGetError();

	// dark blue background for contrast
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

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
	// checking a VR shader
	VRID = LoadShaders("shaders/vr.vertexshader", "shaders/vr.fragmentshader");

	// get a handle for our uniform matrices
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(ProgramID, "View");

	// need a handle for the bounding box one as well
	BoundingMatrixID = glGetUniformLocation(BoundingBoxProgramID, "MVP");

	m_nSceneMatrixLocation = glGetUniformLocation(VRID, "matrix");

	// get a handle for our "texSampler" uniform
	TextureID = glGetUniformLocation(ProgramID, "texSampler");

	// setting up variables for framecounter
	double lastTime = glfwGetTime();
	int numFrames = 0;

	// creating a vector for the texture lib
	std::vector<MtlObj> textureLib;
	// creating k-d tree node;
	kdNode* tree = new kdNode();

	// selecting the model to use
	std::string model = "models/san-miguel";
	
	// if a .bin file exists then load that instead
	if (FILE *file = fopen(std::string(model + ".bin").c_str(), "r"))
	{
		fclose(file);

		// binary file exists for faster loading
		printf("Reading in .bin file...\n");
		readTreeBinaryFile(tree, model);
		readMtlFile(textureLib, model);
	}
	else
	{
		// otherwise load .obj file and index it and check for duplicates
		std::vector<ObjInfo> objInfo;
		std::vector<int> textInd;
		bool res = loadObj(std::string(model + ".obj").c_str(), objInfo, textureLib);

		// creating a vector for the indexed objects
		std::vector<ObjInfo> indexed_objInfo;
		printf("Indexing VBOs...\n");
		vboIndex(objInfo, indexed_objInfo);
		objInfo.clear();

		// create bounding boxes
		printf("Creating bouding boxes...\n");
		createBoundingBox(indexed_objInfo, textureLib);

		// create k-d tree
		printf("Creating k-d tree...\n");
		tree = kdTreeConstruct(indexed_objInfo, 0, (int)indexed_objInfo.size());
		indexed_objInfo.clear();

		// write out binary
		printf("Write out .bin file...\n");
		writeTreeBinaryFile(tree, model);
		writeMtlFile(textureLib, model);
	}

	// if profiling is needed
	/*printf("Profiling...\n");
	profileKdTree(indexed_objInfo);*/

	// load in teaxtures
	std::vector<GLuint> textures;
	printf("Loading textures...\n");
	loadTreeTextures(tree, textureLib, textures);

	// set up the buffers, a buffer per object
	printf("Setting up buffers...\n");
	setupBuffers(tree);

	// generate occlusion query names
	printf("Generate queries...\n");
	generateQueries(tree);

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
	initText2D("textures/cells.png");

	glUseProgram(ProgramID);
	// initilize VR
	printf("Initializing VR...\n");
	VR vr = VR();
	if (USE_VR)
	{
		vr.SetupCameras();
		vr.GetRecommendedRenderTargetSize(vrWidth, vrHeight);
		vr.CreateFrameBuffers(vrWidth, vrHeight, leftEyeDesc);
		vr.CreateFrameBuffers(vrWidth, vrHeight, rightEyeDesc);
	}

	// creating a frustum object
	Frustum frustum;

	// counters
	int bufferPadding = 0;
	int drawCalls = 0;
	int occludedObjects = 0;
	int frustOccluded = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// while the flag to close hasn't been raised the while loop will continue
	while (!glfwWindowShouldClose(window))
	{
		// clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use our shader
		glUseProgram(ProgramID);

		// compute the MVP matrix from keyboard and mouse input
		updateMVP(window, *monitor, g_width, g_height);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		glm::mat4 MVP = ProjectionMatrix * ViewMatrix;

		// updating the view frustum
		if(getVisCheck())
			frustum.update(ProjectionMatrix * ViewMatrix);

		// send transforms to shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0])	;

		// counters
		bufferPadding = 0;
		drawCalls = 0;
		occludedObjects = 0;
		frustOccluded = 0;

		// draw the objects in front to back order
		if (USE_VR)
		{
			RenderStereoTargets(tree, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
				getPosition(), MVP, textureLib, textures, vr);

			vr.submitFramesOpenGL(leftEyeDesc.m_nResolveTextureId, rightEyeDesc.m_nResolveTextureId);

			vr.UpdateHMDMatrixPose();
		}
		else
			frontBack(tree, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
				getPosition(), MVP, textureLib, textures);

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
		sprintf(text, "%d Frustum Culling Enabled", getVisCheck());
		printText2D(text, 10, g_height - 256, 9 * g_width / g_height);
		sprintf(text, "%d Obj Occluded (Frustum)", frustOccluded);
		printText2D(text, 10, g_height - 277, 9 * g_width / g_height);
		sprintf(text, "%d Occlusion Culling Enabled", getOccCheck());
		printText2D(text, 10, g_height - 298, 9 * g_width / g_height);
		sprintf(text, "%d Obj Occluded (Occlusion)", occludedObjects);
		printText2D(text, 10, g_height - 319, 9 * g_width / g_height);
		sprintf(text, "%s", getMode() ? "GL_FILL" : "GL_LINE");
		printText2D(text, 10, g_height - 340, 9 * g_width / g_height);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// turn off vr
	vr.Shutdown();

	// delete queries
	cleanupQueries(tree);

	// cleanup VBO and shader
	cleanupBuffers(tree);
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

