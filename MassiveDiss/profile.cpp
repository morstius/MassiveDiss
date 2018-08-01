#include <stdio.h>
#include <vector>
#include <ctime>
#include <chrono>
#include <string>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "kdTree.h"

#include "common/types/ObjInfo.h"


void generateTreeAndWriteResults(FILE *outFile, std::vector<ObjInfo> objInfo)
{
	// start timer
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	// generate tree
	kdNode *tree = kdTreeConstruct(objInfo, 0, (int)objInfo.size());

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

	fprintf(outFile, "=====================================================\n");
	fprintf(outFile, "ObjInfo size: %zd\n", objInfo.size());

	fprintf(outFile, "Data size: \n");

	int vertices = 0, indices = 0, normals = 0, uvs = 0;
	int verCount = 0, indCount = 0, normCount = 0, uvCount = 0;

	for (int i = 0; i < objInfo.size(); ++i)
	{
		vertices += sizeof(objInfo[i].vertices) + sizeof(glm::vec3) * (int)objInfo[i].vertices.capacity();
		indices += sizeof(objInfo[i].indices) + sizeof(unsigned int) * (int)objInfo[i].indices.capacity();
		normals += sizeof(objInfo[i].normals) + sizeof(glm::vec3) * (int)objInfo[i].normals.capacity();
		uvs += sizeof(objInfo[i].uvs) + sizeof(glm::vec2) * (int)objInfo[i].uvs.capacity();

		verCount += (int)objInfo[i].vertices.size();
		indCount += (int)objInfo[i].indices.size();
		normCount += (int)objInfo[i].normals.size();
		uvCount += (int)objInfo[i].uvs.size();
	}

	fprintf(outFile, "\n-------------------- NEW ELEMENT ---------------------\n");
	fprintf(outFile, "%zd Vertices (glm::vec3) takes up %zd bytes\n", objInfo[objInfo.size() - 1].vertices.size(), 
		sizeof(objInfo[objInfo.size() - 1].vertices) + sizeof(glm::vec3) * objInfo[objInfo.size() - 1].vertices.capacity());
	fprintf(outFile, "%zd Indices (unsigned int) takes up %zd bytes\n", objInfo[objInfo.size() - 1].indices.size(),
		sizeof(objInfo[objInfo.size() - 1].indices) + sizeof(unsigned int) * objInfo[objInfo.size() - 1].indices.capacity());
	fprintf(outFile, "%zd Normals (glm::vec3) takes up %zd bytes\n", objInfo[objInfo.size() - 1].normals.size(),
		sizeof(objInfo[objInfo.size() - 1].normals) + sizeof(glm::vec3) * objInfo[objInfo.size() - 1].normals.capacity());
	fprintf(outFile, "%zd UVs (glm::vec2) takes up %zd bytes\n", objInfo[objInfo.size() - 1].uvs.size(),
		sizeof(objInfo[objInfo.size() - 1].uvs) + sizeof(glm::vec2) * objInfo[objInfo.size() - 1].uvs.capacity());
	fprintf(outFile, "Texture index of additional %zd bytes\n", sizeof(int));

	fprintf(outFile, "\n----------------------- TOTAL ------------------------\n");
	fprintf(outFile, "%d Vertices (glm::vec3) takes up %d bytes\n", verCount, vertices);
	fprintf(outFile, "%d Indices (unsigned int) takes up %d bytes\n", indCount, indices);
	fprintf(outFile, "%d Normals (glm::vec3) takes up %d bytes\n", normCount, normals);
	fprintf(outFile, "%d UVs (glm::vec2) takes up %d bytes\n", uvCount, uvs);

	int texSize = (int)objInfo.size() * sizeof(int);
	fprintf(outFile, "We have %zd objInfo elements and each one has a texture index (int) total %d bytes\n", objInfo.size(), texSize);

	fprintf(outFile, "\n=====================   TIME  =======================\n");
	long sum = vertices + indices + normals + uvs + texSize;
	fprintf(outFile, "Total size %d bytes\n", sum);

	fprintf(outFile, "Creating k-d tree took %f seconds\n", time_span.count());
	fprintf(outFile, "Creating k-d tree spent %f seconds to sorting\n", tree->time_sorting.count());
	fprintf(outFile, "Creating k-d tree spent %f creating leaf nodes\n", tree->time_creatingLeafNode.count());
	fprintf(outFile, "Creating k-d tree spent %f seconds internal nodes\n", tree->time_creatingIntNode.count());
	fprintf(outFile, "=====================================================\n");
}


void profileKdTree(std::vector<ObjInfo> objInfo)
{
	// creating distinct file name
	std::string fileName = "profile/profile_" + std::to_string(objInfo.size()) + ".txt";

	// opening output file
	FILE *outFile = fopen(fileName.c_str(), "w");

	fprintf(outFile, "======================= Profiling =======================\n");
	
	for (int i = 1; i < objInfo.size(); i++)
	{
		fprintf(outFile, "\n");

		std::vector<ObjInfo>::const_iterator first = objInfo.begin();
		std::vector<ObjInfo>::const_iterator last = objInfo.begin() + i;
		std::vector<ObjInfo> newVec(first, last);

		generateTreeAndWriteResults(outFile, newVec);

		fprintf(outFile, "\n\n");
	}


	// closing output file
	fclose(outFile);
}
