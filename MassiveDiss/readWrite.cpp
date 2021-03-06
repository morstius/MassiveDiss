#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>
#include <string>

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include "common/types/ObjInfo.h"
#include "common/types/MtlObj.h"

#include "kdTree.h"

void writeFile(const std::vector<ObjInfo>& objInfo, const std::vector<MtlObj>& textureLib, std::string name)
{
	FILE* file;

	// open file
	file = fopen(std::string(name + ".objs").c_str(), "w");

	fprintf(file, "# preprocessed .obj file\n");

	// write each object in vector
	for (int i = 0; i < objInfo.size(); ++i)
	{
		if (objInfo[i].vertices.size() < 1 || objInfo[i].indices.size() < 1)
			continue;

		fprintf(file, "o obj %d\n", i);
		fprintf(file, "t %s\n", textureLib[objInfo[i].txIdx].map_Kd.c_str());
		for (int j = 0; j < objInfo[i].vertices.size(); ++j)
		{
			fprintf(file, "v %f %f %f\n", objInfo[i].vertices[j].x, objInfo[i].vertices[j].y, objInfo[i].vertices[j].z);
			fprintf(file, "vt %f %f\n", objInfo[i].uvs[j].x, objInfo[i].uvs[j].y);
			fprintf(file, "vn %f %f %f\n", objInfo[i].normals[j].x, objInfo[i].normals[j].y, objInfo[i].normals[j].z);
		}

		for (int j = 0; j < objInfo[i].boundingVert.size(); ++j)
		{
			fprintf(file, "b %f %f %f\n", objInfo[i].boundingVert[j].x, objInfo[i].boundingVert[j].y, objInfo[i].boundingVert[j].z);
		}

		for (int j = 0; j < objInfo[i].indices.size(); ++j)
		{
			fprintf(file, "i %d\n", objInfo[i].indices[j]);
		}
	}

	// close file
	fclose(file);
}

void writeBinaryFile(const std::vector<ObjInfo>& objInfo, std::string name)
{
	// open file
	std::ofstream file;
	file.open(std::string(name + ".bin"), std::ios::binary);

	unsigned size = objInfo.size();
	file.write((char*)&size, sizeof(unsigned));

	// write each object in vector
	for (int i = 0; i < objInfo.size(); ++i)
	{
		if (objInfo[i].vertices.size() < 1 || objInfo[i].indices.size() < 1)
			continue;

		// vertices, uvs and normals
		unsigned vertSize = objInfo[i].vertices.size();
		file.write(reinterpret_cast<char *>(&vertSize), sizeof(unsigned));
		std::vector <glm::vec3> vec = objInfo[i].vertices;
		file.write(reinterpret_cast<char *>(&vec[0]), objInfo[i].vertices.size() * sizeof(glm::vec3));
		std::vector <glm::vec2> uv = objInfo[i].uvs;
		file.write(reinterpret_cast<char *>(&uv[0]), objInfo[i].uvs.size() * sizeof(glm::vec2));
		std::vector <glm::vec3> norm = objInfo[i].normals;
		file.write(reinterpret_cast<char *>(&norm[0]), objInfo[i].normals.size() * sizeof(glm::vec3));

		// bounding box vertices
		unsigned boundVertSize = objInfo[i].boundingVert.size();
		file.write(reinterpret_cast<char *>(&boundVertSize), sizeof(unsigned));

		std::vector <glm::vec3> bb = objInfo[i].boundingVert;
		file.write(reinterpret_cast<char *>(&bb[0]), objInfo[i].boundingVert.size() * sizeof(glm::vec3));

		// indices
		unsigned idxSize = objInfo[i].indices.size();
		file.write(reinterpret_cast<char *>(&idxSize), sizeof(unsigned));

		std::vector <unsigned int> indices = objInfo[i].indices;
		file.write(reinterpret_cast<char *>(&indices[0]), objInfo[i].indices.size() * sizeof(unsigned int));

		// texture
		int txdInd = objInfo[i].txIdx;
		file.write(reinterpret_cast<char *>(&txdInd), sizeof(int));

		// center information
		glm::vec3 center = objInfo[i]._center;
		file.write(reinterpret_cast<char *>(&center[0]), sizeof(glm::vec3));

		// occlusion information
		bool occlusion = objInfo[i].useForOcclusion;
		file.write(reinterpret_cast<char *>(&occlusion), sizeof(bool));
	}

	// close file
	file.close();
}

void writeNode(std::ofstream& file, kdNode* tree)
{
	// bool leaf
	bool leaf = tree->isLeaf;
	file.write(reinterpret_cast<char *>(&tree->isLeaf), sizeof(bool));

	if (tree->isLeaf)
	{
		unsigned size = tree->objInfo.size();
		file.write((char*)&size, sizeof(unsigned));

		for (int i = 0; i < size; ++i)
		{
			// vertices, uvs and normals
			unsigned vertSize = tree->objInfo[i].vertices.size();
			file.write(reinterpret_cast<char *>(&vertSize), sizeof(unsigned));
			std::vector <glm::vec3> vec = tree->objInfo[i].vertices;
			file.write(reinterpret_cast<char *>(&vec[0]), tree->objInfo[i].vertices.size() * sizeof(glm::vec3));
			std::vector <glm::vec2> uv = tree->objInfo[i].uvs;
			file.write(reinterpret_cast<char *>(&uv[0]), tree->objInfo[i].uvs.size() * sizeof(glm::vec2));
			std::vector <glm::vec3> norm = tree->objInfo[i].normals;
			file.write(reinterpret_cast<char *>(&norm[0]), tree->objInfo[i].normals.size() * sizeof(glm::vec3));

			// bounding box vertices
			unsigned boundVertSize = tree->objInfo[i].boundingVert.size();
			file.write(reinterpret_cast<char *>(&boundVertSize), sizeof(unsigned));

			std::vector <glm::vec3> bb = tree->objInfo[i].boundingVert;
			file.write(reinterpret_cast<char *>(&bb[0]), tree->objInfo[i].boundingVert.size() * sizeof(glm::vec3));

			// indices
			unsigned idxSize = tree->objInfo[i].indices.size();
			file.write(reinterpret_cast<char *>(&idxSize), sizeof(unsigned));

			std::vector <unsigned int> indices = tree->objInfo[i].indices;
			file.write(reinterpret_cast<char *>(&indices[0]), tree->objInfo[i].indices.size() * sizeof(unsigned int));

			// texture
			int txdInd = tree->objInfo[i].txIdx;
			file.write(reinterpret_cast<char *>(&txdInd), sizeof(int));

			// center information
			glm::vec3 center = tree->objInfo[i]._center;
			file.write(reinterpret_cast<char *>(&center[0]), sizeof(glm::vec3));

			// occlusion information
			bool occlusion = tree->objInfo[i].useForOcclusion;
			file.write(reinterpret_cast<char *>(&occlusion), sizeof(bool));
		}
	}
	else
	{
		// where the split was made
		float split = tree->split;
		file.write(reinterpret_cast<char *>(&split), sizeof(float));

		// which axis was the split on
		int axis = tree->axis;

		writeNode(file, tree->left);
		writeNode(file, tree->right);
	}
}

void writeTreeBinaryFile(kdNode* tree, std::string name)
{
	// open file
	std::ofstream file;
	file.open(std::string(name + ".bin"), std::ios::binary);

	// go through node by node
	writeNode(file, tree);

	// close file
	file.close();
}

void readFile(std::vector<ObjInfo>& objInfo, std::vector<MtlObj>& textureLib, std::string name)
{
	std::ifstream file;

	// open file in input mode
	file.open(std::string(name + ".objs").c_str(), std::ios::in);

	char line[256];
	bool first = true;

	ObjInfo oi;
	int txIndex = 0;
	while (file.getline(line, 256))
	{
		switch (line[0])
		{
			// a dummy for the first letters
			char dummy[10];

			case 'v':
			{
				switch (line[1])
				{
					case 't':
					{
						// uv
						glm::vec2 vec;
						sscanf(line, "%s %f %f", dummy, &(vec.x), &(vec.y));
						oi.uvs.push_back(vec);
						break;
					}
					case 'n':
					{
						// normal
						glm::vec3 vec;
						sscanf(line, "%s %f %f %f", dummy, &(vec.x), &(vec.y), &(vec.z));
						oi.normals.push_back(vec);
						break;
					}
					default:
					{
						// vertex
						glm::vec3 vec;
						sscanf(line, "%s %f %f %f", dummy, &(vec.x), &(vec.y), &(vec.z));
						oi.vertices.push_back(vec);
						break;
					}
				}
				break;
			}
			case 'o':
			{
				if (first)
				{
					first = false;
					break;
				}
				objInfo.push_back(oi);
				oi = ObjInfo();
				break;
			}
			case 't':
			{
				// index
				char path[255];
				int match = sscanf(line, "%s %s", dummy, &path);

				if (match == 2)
				{
					oi.txIdx = txIndex++;

					MtlObj mtl;
					mtl.map_Kd = path;
					mtl.hasTexture = true;
					textureLib.push_back(mtl);
				}
				else
				{
					oi.txIdx = -1;
				}
				break;
			}
			case 'i':
			{
				// index
				int idx;
				sscanf(line, "%s %d", dummy, &idx);
				oi.indices.push_back(idx);
				break;
			}
			case 'b':
			{
				// boundingbox
				glm::vec3 vec;
				sscanf(line, "%s %f %f %f", dummy, &(vec.x), &(vec.y), &(vec.z));
				oi.boundingVert.push_back(vec);
				break;
			}
		}
	}

	objInfo.push_back(oi);

	// close file
	file.close();
}

void readBinaryFile(std::vector<ObjInfo>& objInfo, const std::string& name)
{
	int texId = 0;

	std::ifstream file;

	// open file in input mode
	file.open(std::string(name + ".bin"), std::ios::binary);

	unsigned  size;
	file.read((char*)&size, sizeof(unsigned));
	
	// read in
	for (int i = 0; i < size; ++i)
	{
		ObjInfo oi;

		// vertices, uvs, normals
		unsigned vertSize;
		file.read(reinterpret_cast<char *>(&vertSize), sizeof(unsigned));
		oi.vertices.resize(vertSize);
		file.read(reinterpret_cast<char *>(&oi.vertices[0]), vertSize * sizeof(glm::vec3));
		oi.uvs.resize(vertSize);
		file.read(reinterpret_cast<char *>(&oi.uvs[0]), vertSize * sizeof(glm::vec2));
		oi.normals.resize(vertSize);
		file.read(reinterpret_cast<char *>(&oi.normals[0]), vertSize * sizeof(glm::vec3));

		// bounding box vertices
		unsigned boundVertSize;
		file.read(reinterpret_cast<char *>(&boundVertSize), sizeof(unsigned));
		oi.boundingVert.resize(boundVertSize);
		file.read(reinterpret_cast<char *>(&oi.boundingVert[0]), boundVertSize * sizeof(glm::vec3));

		// indices
		unsigned idxSize;
		file.read(reinterpret_cast<char *>(&idxSize), sizeof(unsigned));
		oi.indices.resize(idxSize);
		file.read(reinterpret_cast<char *>(&oi.indices[0]), idxSize * sizeof(unsigned int));

		// texture
		int txdInd;
		file.read(reinterpret_cast<char *>(&txdInd), sizeof(int));
		oi.txIdx = txdInd;

		// center information
		glm::vec3 center;
		file.read(reinterpret_cast<char *>(&center[0]), sizeof(glm::vec3));
		oi._center = center;

		// occlusion information
		bool occlusion;
		file.read(reinterpret_cast<char *>(&occlusion), sizeof(bool));
		oi.useForOcclusion = occlusion;

		objInfo.push_back(oi);
	}

	file.close();
}

void readNode(std::ifstream& file, kdNode* tree)
{
	// bool leaf
	bool leaf;
	file.read(reinterpret_cast<char *>(&tree->isLeaf), sizeof(bool));

	if (tree->isLeaf)
	{
		unsigned size = tree->objInfo.size();
		file.read((char*)&size, sizeof(unsigned));

		for (int i = 0; i < size; ++i)
		{
			ObjInfo oi;

			// vertices, uvs, normals
			unsigned vertSize;
			file.read(reinterpret_cast<char *>(&vertSize), sizeof(unsigned));
			oi.vertices.resize(vertSize);
			file.read(reinterpret_cast<char *>(&oi.vertices[0]), vertSize * sizeof(glm::vec3));
			oi.uvs.resize(vertSize);
			file.read(reinterpret_cast<char *>(&oi.uvs[0]), vertSize * sizeof(glm::vec2));
			oi.normals.resize(vertSize);
			file.read(reinterpret_cast<char *>(&oi.normals[0]), vertSize * sizeof(glm::vec3));

			// bounding box vertices
			unsigned boundVertSize;
			file.read(reinterpret_cast<char *>(&boundVertSize), sizeof(unsigned));
			oi.boundingVert.resize(boundVertSize);
			file.read(reinterpret_cast<char *>(&oi.boundingVert[0]), boundVertSize * sizeof(glm::vec3));

			// indices
			unsigned idxSize;
			file.read(reinterpret_cast<char *>(&idxSize), sizeof(unsigned));
			oi.indices.resize(idxSize);
			file.read(reinterpret_cast<char *>(&oi.indices[0]), idxSize * sizeof(unsigned int));

			// texture
			int txdInd;
			file.read(reinterpret_cast<char *>(&txdInd), sizeof(int));
			oi.txIdx = txdInd;

			// center information
			glm::vec3 center;
			file.read(reinterpret_cast<char *>(&center[0]), sizeof(glm::vec3));
			oi._center = center;

			// occlusion information
			bool occlusion;
			file.read(reinterpret_cast<char *>(&occlusion), sizeof(bool));
			oi.useForOcclusion = occlusion;

			tree->objInfo.push_back(oi);
			tree->isLeaf = true;
		}
	}
	else
	{
		// where the split was made
		float split;
		file.read(reinterpret_cast<char *>(&split), sizeof(float));

		// which axis was the split on
		int axis = tree->axis;

		tree->left = new kdNode();
		tree->right = new kdNode();
		readNode(file, tree->left);
		readNode(file, tree->right);
	}
}

void readTreeBinaryFile(kdNode* tree, const std::string name)
{
	int texId = 0;

	std::ifstream file;

	// open file in input mode
	file.open(std::string(name + ".bin"), std::ios::binary);

	// go through node by node
	readNode(file, tree);

	file.close();
}

void writeMtlFile(const std::vector<MtlObj>& mtlObj, const std::string& name)
{
	FILE* file;

	// open file
	file = fopen(std::string(name + ".mtls").c_str(), "w");

	for (int i = 0; i < mtlObj.size(); ++i)
	{
		int len = strlen(mtlObj[i].map_Kd.c_str());
		if(len > 0)
			fprintf(file, "%s\n", mtlObj[i].map_Kd.c_str());
		else
			fprintf(file, "%s\n", "x");
	}

	fclose(file);
}

void readMtlFile(std::vector<MtlObj>& mtlObj, const std::string& name)
{
	std::ifstream file;

	// open file in input mode
	file.open(std::string(name + ".mtls").c_str(), std::ios::in);

	char line[256];

	while (file.getline(line, 256))
	{
		char path[100];
		sscanf(line, "%s", &path);

		MtlObj mtl;
		if (strlen(path) == 1)
		{
			mtl.hasTexture = false;
			mtl.map_Kd = path;
		}
		else
		{
			mtl.hasTexture = true;
			mtl.map_Kd = path;
		}

		mtlObj.push_back(mtl);
	}

	file.close();
}