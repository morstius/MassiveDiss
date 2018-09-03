#include "occlusionCheck.h"

#include <string>
#include <random>
#include <unordered_map>

#include "kdTree.h"

#include "common/controls.h"

#include "common/types/ObjInfo.h"
#include "common/types/MtlObj.h"

void check(
	std::unordered_map<std::string, int>& occlusionCounter,
	ObjInfo& objInfo,
	Frustum& frustum,
	int& bufferPadding,
	int& occludedObjects,
	int& frustOccluded,
	int& drawCalls,
	glm::mat4& MVP,
	std::vector<MtlObj>& textureLib,
	std::vector<GLuint>& textures,
	GLuint BoundingBoxProgramID,
	GLuint BoundingMatrixID,
	GLuint ProgramID,
	GLuint TextureID,
	GLuint MatrixID,
	GLuint ViewMatrixID
)
{
	// if using frustum culling and the bounding sphere is not inside the frustum then it is not drawn
	if (!frustum.checkSphere(objInfo.center(), objInfo.size()))
	{
		std::string center = "(" + std::to_string(objInfo.center().x) + "," +
			std::to_string(objInfo.center().y) + "," +
			std::to_string(objInfo.center().z) + ")";
		auto search = occlusionCounter.find(center);
		if (search != occlusionCounter.end())
		{
			search->second++;
		}
		else
		{
			occlusionCounter[center] = 1;
		}
	}
}

void frontBack(
	std::unordered_map<std::string, int>& occlusionCounter,
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
	GLuint BoundingBoxProgramID,
	GLuint BoundingMatrixID,
	GLuint ProgramID,
	GLuint TextureID,
	GLuint MatrixID,
	GLuint ViewMatrixID
)
{
	if (tree->isLeaf)
	{
		// reached a leaf so I have some objects to draw
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			check(occlusionCounter, tree->objInfo[i], frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
				ProgramID, TextureID, MatrixID, ViewMatrixID);
		}
	}
	else
	{
		// ordering which brach to go down first
		if (tree->axis == kdNode::axisSplit::xaxis)
		{
			if (pos.x < tree->split)
			{
				frontBack(occlusionCounter, tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
				frontBack(occlusionCounter, tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
			}
			else
			{
				frontBack(occlusionCounter, tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
				frontBack(occlusionCounter, tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
			}
		}
		if (tree->axis == kdNode::axisSplit::zaxis)
		{
			if (pos.z < tree->split)
			{
				frontBack(occlusionCounter, tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
				frontBack(occlusionCounter, tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
			}
			else
			{
				frontBack(occlusionCounter, tree->right, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
				frontBack(occlusionCounter, tree->left, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
					pos, MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID,
					ProgramID, TextureID, MatrixID, ViewMatrixID);
			}
		}
	}
}

void findMaxMin(kdNode* tree, 
	float& minX, float& maxX,
	float& minY, float& maxY, 
	float& minZ, float& maxZ
)
{
	if (tree->isLeaf)
	{
		for (int i = 0; i < tree->objInfo.size(); ++i)
		{
			float minXt, maxXt, minYt, maxYt, minZt, maxZt;

			tree->objInfo[i].getMinMax(minXt, maxXt, minYt, maxYt, minZt, maxZt);

			if (minXt < minX)
				minX = minXt;
			if (maxXt > maxX)
				maxX = maxXt;

			if (minYt < minY)
				minY = minYt;
			if (maxYt > maxY)
				maxY = maxYt;

			if (minZt < minZ)
				minZ = minZt;
			if (maxZt > maxZ)
				maxZ = maxZt;
		}
	}
	else
	{
		// recursively find min and max for x, y and z
		findMaxMin(tree->left, minX, maxX, minY, maxY, minZ, maxZ);
		findMaxMin(tree->right, minX, maxX, minY, maxY, minZ, maxZ);
	}
}

void randomLocation(
	kdNode* tree,
	glm::mat4& MVP,
	float& minX,
	float& maxX,
	float& minY,
	float& maxY,
	float& minZ,
	float& maxZ
)
{
	std::default_random_engine rndEngine((unsigned)time(nullptr));

	// finding the area I want to be in
	if(minX == 0 && maxX == 0)
		findMaxMin(tree, minX, maxX, minY, maxY, minZ, maxZ);

	std::uniform_real_distribution<float> rndX(minX, maxX);
	std::uniform_real_distribution<float> rndY(minY, maxY);
	std::uniform_real_distribution<float> rndZ(minZ, maxZ);

	float x, y, z;

	float horizontal, vertical;
	std::uniform_real_distribution<float> rndAngle(0, 360);

	std::unordered_map<std::string, int> frustumOcclusionCounter;
	std::unordered_map<std::string, int> occlusionCounter;

	x = rndX(rndEngine);
	y = rndY(rndEngine);
	z = rndZ(rndEngine);

	horizontal = rndAngle(rndEngine);
	vertical = rndAngle(rndEngine);

	// update, so we have the right MVP
	setPosition(glm::vec3(x, y, z));

	setHorizontal(horizontal);
	setVertical(vertical);

	update();

	// compute the MVP matrix from keyboard and mouse input
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();

	MVP = ProjectionMatrix * ViewMatrix;
}

void makeOcclusionCheck(
	GLFWwindow* window,
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
	GLuint BoundingBoxProgramID,
	GLuint BoundingMatrixID,
	GLuint ProgramID,
	GLuint TextureID,
	GLuint MatrixID,
	GLuint ViewMatrixID
)
{
	// want to pick a random location in the scene and a random direction
	// draw the scene, perform the occlusion query and check what is rendered and what is occluded

	// creating distinct file name
	std::string fileName = "occlusion/occlusionInfo.txt";

	// opening output file
	FILE *outFile = fopen(fileName.c_str(), "w");

	fprintf(outFile, "##########################################################\n");
	fprintf(outFile, "##################### Occlusion check ####################\n");
	fprintf(outFile, "##########################################################\n\n");

	std::default_random_engine rndEngine((unsigned)time(nullptr));

	// finding the area I want to be in
	float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f, minZ = 0.0f, maxZ = 0.0f;
	findMaxMin(tree, minX, maxX, minY, maxY, minZ, maxZ);

	std::uniform_real_distribution<float> rndX(minX, maxX);
	std::uniform_real_distribution<float> rndY(minY, maxY);
	std::uniform_real_distribution<float> rndZ(minZ, maxZ);

	float x, y, z;

	float horizontal, vertical;
	std::uniform_real_distribution<float> rndAngle(0, 360);

	std::unordered_map<std::string, int> frustumOcclusionCounter;

	int frustumIterations = 1000;

	fprintf(outFile, "%zd different frustum checks.\n\n", frustumIterations);

	// loop through 100 variations and checking what objects are culled through frustum culling at each time
	for (int i = 0; i < frustumIterations; ++i)
	{
		x = rndX(rndEngine);
		y = rndY(rndEngine);
		z = rndZ(rndEngine);

		horizontal = rndAngle(rndEngine);
		vertical = rndAngle(rndEngine);

		// update, so we have the right MVP
		setPosition(glm::vec3(x, y, z));

		setHorizontal(horizontal);
		setVertical(vertical);

		update();

		// compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		MVP = ProjectionMatrix * ViewMatrix;

		// updating the view frustum
		frustum.update(MVP);

		frontBack(frustumOcclusionCounter, tree, frustum, bufferPadding, occludedObjects, frustOccluded, drawCalls,
			getPosition(), MVP, textureLib, textures, BoundingBoxProgramID, BoundingMatrixID, ProgramID, TextureID, MatrixID, ViewMatrixID);
	}

	fprintf(outFile, "################################ FRUSTUM CULLING ##########################################\n");

	// write out frustum information
	for (auto it = frustumOcclusionCounter.begin(); it != frustumOcclusionCounter.end(); ++it)
	{
		fprintf(outFile, "The object with it's center at %s was culled %zd times\n", it->first.c_str(), it->second);
	}

	fprintf(outFile, "\n\n################################ OCCLUSION CULLING ##########################################\n");

	fclose(outFile);
}

void checkOcclusion(kdNode* tree, std::unordered_map<std::string, int>& counter)
{
	if (tree->isLeaf)
	{
		// if using frustum culling and the bounding sphere is not inside the frustum then it is not drawn
		if (tree->objInfo[0].occluded)
		{
			std::string center = "(" + std::to_string(tree->objInfo[0].center().x) + "," +
				std::to_string(tree->objInfo[0].center().y) + "," +
				std::to_string(tree->objInfo[0].center().z) + ")";
			auto search = counter.find(center);
			if (search != counter.end())
			{
				search->second++;
			}
			else
			{
				counter[center] = 1;
			}

			tree->objInfo[0].occluded = false;
		}
	}
	else
	{
		checkOcclusion(tree->left, counter);
		checkOcclusion(tree->right, counter);
	}
}

void printOcclusion(kdNode* tree, std::unordered_map<std::string, int> occlusionCounter)
{
	// creating distinct file name
	std::string fileName = "occlusion/occlusionInfo.txt";

	// opening output file
	FILE *outFile = fopen(fileName.c_str(), "a");

	// write out frustum information
	for (auto it = occlusionCounter.begin(); it != occlusionCounter.end(); ++it)
	{
		fprintf(outFile, "The object with it's center at %s was culled %zd times\n", it->first.c_str(), it->second);
	}

	fclose(outFile);
}