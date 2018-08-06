#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <string>
#include <map>

#include <glm/glm.hpp>

#include "ModelLoader.hpp"
#include "common/types/MtlObj.h"
#include "common/types/ObjInfo.h"
#include "common/types/IdxInfo.h"

int findTexIdx(
	std::string curTxt,
	std::vector<MtlObj>& txtLib
)
{
	//find the index of the texture in the texture lib
	int idx = -1;
	for (int i = 0; i < txtLib.size(); i++)
	{
		// checking if texture is already in the lib
		if (curTxt.compare(txtLib[i].newmtl) == 0)
			return i;
	}

	// printing out error if nothing was found, shouldn't happens
	printf("Texture not found!!!\n");

	return idx;
}

// read the material file and create a texture lib object with texture paths
bool populateMtlLib(
	std::string filepath,
	std::vector<MtlObj>& textureLib
)
{
	std::string path = "models/" + filepath;

	// open file
	std::ifstream inputFile(path, std::ios::in);

	// making sure that file is open
	if (!inputFile.is_open()) {
		printf("Can't open file %s!\n", path.c_str());
		getchar();
		return false;
	}

	// create new material object
	MtlObj mtlObj = MtlObj();

	// initialize variables
	char line[256];
	bool hasText = false;

	// some helping variables
	std::string newmtl = "none";
	std::string map_Kd = "";

	while (inputFile.getline(line, 256))
	{
		// a dummy for the first letters
		char dummy[10];

		char objName[50];
		sscanf(line, "%s %s", &dummy, &objName);

		if (strcmp(dummy, "map_Kd") == 0)
		{
			// populate the texture path
			std::string textPath = "textures/" + std::string(objName);

			map_Kd = std::string(textPath);
			hasText = true;
		}

		if (strcmp(dummy, "newmtl") == 0)
		{
			// new material type
			if (newmtl.compare("none") != 0)
			{
				hasText = map_Kd.compare("") == 0 ? false : true;
				textureLib.push_back(MtlObj(newmtl, map_Kd, hasText));

				map_Kd = "";
			}

			newmtl = std::string(objName);
		}
	}

	// if there is no texture then setting the path as an empty string
	if (!hasText)
		map_Kd = std::string("");

	// need very little info, just the path and that it is in the right order
	textureLib.push_back(MtlObj(newmtl, map_Kd, hasText));

	return 0;
}

bool loadObj(
	const char * path,
	std::vector<ObjInfo>& out_objInfo,
	std::vector<MtlObj>& textureLib
)
{
	printf("Loading .obj file %s...\n",	path);

	// some helping and temporary variables
	std::vector<IdxInfo> idxInfo;
	std::vector<unsigned int> vertexIdx, uvIdx, normalIdx;
	std::vector<glm::vec3> temp_vert, temp_norm;
	std::vector<glm::vec2> temp_uvs;
	std::vector<int> temp_texInd;

	// open file
	std::ifstream inputFile(path, std::ios::in);

	// making sure the file is open
	if (!inputFile.is_open()) {
		printf("Can't open the file %s!\n", path);
		getchar();
		return false;
	}

	char line[256];
	char curText[50];
	bool loadMtl = false;

	while (inputFile.getline(line, 256))
	{
		// the first letter of each line tells us what to do with the rest of the line
		switch (line[0])
		{
			// a dummy for the first letters
			char dummy[10];

			case 'v':
			{
				switch (line[1])
				{
					// finding which array to put the info into
					case ' ':
					{
						// dummy for values
						glm::vec3 vec;

						sscanf(line, "%s %f %f %f", dummy, &(vec.x), &(vec.y), &(vec.z));
						temp_vert.push_back(vec);
						break;
					}
					case 'n':
					{
						// dummy for values
						glm::vec3 vec;

						sscanf(line, "%s %f %f %f", dummy, &(vec.x), &(vec.y), &(vec.z));
						temp_norm.push_back(vec);
						break;
					}
					case 't':
					{
						// dummy for values
						glm::vec2 vec;

						sscanf(line, "%s %f %f", dummy, &(vec.x), &(vec.y));
						vec.y *= -1;
						temp_uvs.push_back(vec);
						break;
					}
				}
				break;
			}
			case 'f':
			{
				// some of the .obj files had quads instead of triangles so this was made since it wasn't too much extra
				unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
				int matches = sscanf(line, "%s %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", dummy, 
					&vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3]);

				// need to check how many matches were made to know if it has triangles or quads
				switch (matches)
				{
					// case 10 is reading in a triangle
					case 10:
					{
						// set vertex indices
						vertexIdx.push_back(vertexIndex[0]);
						vertexIdx.push_back(vertexIndex[1]);
						vertexIdx.push_back(vertexIndex[2]);
						// set uv indices
						uvIdx.push_back(uvIndex[0]);
						uvIdx.push_back(uvIndex[1]);
						uvIdx.push_back(uvIndex[2]);
						// set normal indices
						normalIdx.push_back(normalIndex[0]);
						normalIdx.push_back(normalIndex[1]);
						normalIdx.push_back(normalIndex[2]);
						
						break;
					}
					// case 13 is reading in a quad
					case 13:
					{
						// set vertex indices
						vertexIdx.push_back(vertexIndex[0]);
						vertexIdx.push_back(vertexIndex[1]);
						vertexIdx.push_back(vertexIndex[2]);
						// set uv indices
						uvIdx.push_back(uvIndex[0]);
						uvIdx.push_back(uvIndex[1]);
						uvIdx.push_back(uvIndex[2]);
						// set normal indices
						normalIdx.push_back(normalIndex[0]);
						normalIdx.push_back(normalIndex[1]);
						normalIdx.push_back(normalIndex[2]);

						// set vertex indices
						vertexIdx.push_back(vertexIndex[2]);
						vertexIdx.push_back(vertexIndex[3]);
						vertexIdx.push_back(vertexIndex[0]);
						// set uv indices
						uvIdx.push_back(uvIndex[2]);
						uvIdx.push_back(uvIndex[3]);
						uvIdx.push_back(uvIndex[0]);
						// set normal indices
						normalIdx.push_back(normalIndex[2]);
						normalIdx.push_back(normalIndex[3]);
						normalIdx.push_back(normalIndex[0]);

						break;
					}
					default:
					{
						// default which shouldn't happen
						printf("Obj format not as expected.\n");
						inputFile.close();
						return false;
					}
				}
				

				break;
			}
			case 'o':
			{
				char objName[256];
				sscanf(line, "%s %s", dummy, &objName);

				// create object to add onto in the vector 
				if (vertexIdx.size() != 0)
				{
					IdxInfo ii = IdxInfo();
					ii.normalIndices = normalIdx;
					ii.uvIndices = uvIdx;
					ii.vertexIndices = vertexIdx;
					ii.textureIndex = findTexIdx(curText, textureLib);
					
					idxInfo.push_back(ii);

					vertexIdx.clear();
					normalIdx.clear();
					uvIdx.clear();
				}

				break;
			}
			case 'm':
			{
				// load the material lib file
				char objName[50];
				sscanf(line, "%s %s", &dummy, &objName);
				if (strcmp(dummy, "mtllib") == 0)
				{
					// this is the material lib link
					// I only expect one material lib file so it only reads the first one it finds
					if (!loadMtl)
					{
						printf("Loading material lib...\n");
						populateMtlLib(std::string(objName), textureLib);
						loadMtl = true;
					}
				}
				break;
			}
			case 'u':
			{
				// set the right material per object
				char objName[256];
				sscanf(line, "%s %s", &dummy, &objName);

				// for some reason the value kept changing each time it entered here, until I did this
				// works so I leave it
				if (strcmp(dummy, "usemtl") == 0)
				{
					strcpy(curText, objName);
				}
				break;
			}
		}
	}

	//add the last idx info to the vector
	IdxInfo ii = IdxInfo();
	ii.normalIndices = normalIdx;
	ii.uvIndices = uvIdx;
	ii.vertexIndices = vertexIdx;
	ii.textureIndex = findTexIdx(curText, textureLib);

	idxInfo.push_back(ii);

	// remember to be nice and close the file
	inputFile.close();

	// go through each object and load attributes in the right order
	// according to the face part in the .obj file
	for (unsigned int j = 0; j < idxInfo.size(); j++)
	{
		ObjInfo oi = ObjInfo();
		for (unsigned int i = 0; i < idxInfo[j].vertexIndices.size(); i++)
		{
			// get the indices of its attributes
			unsigned int vertexIndex = idxInfo[j].vertexIndices[i];
			unsigned int uvIndex = idxInfo[j].uvIndices[i];
			unsigned int normalIndex = idxInfo[j].normalIndices[i];

			// add them to the vectors in the right order
			oi.vertices.push_back(temp_vert[vertexIndex - 1]);
			oi.uvs.push_back(temp_uvs[uvIndex - 1]);
			oi.normals.push_back(temp_norm[normalIndex - 1]);
			
			// also need the texture index
			oi.txIdx = idxInfo[j].textureIndex;
		}
		out_objInfo.push_back(oi);
	}

	return 0;
}

// a helping class to allow me to use the map function more easily,
// since using find() is faster than a for loop
struct attributeCombo
{
	// constructor
	attributeCombo(glm::vec3 v, glm::vec2 u, glm::vec3 n)
		: vertex(v), uv(u), normal(n)
	{}

	~attributeCombo()
	{}

	// overwriting the compare operator
	bool operator==(const attributeCombo ac) const
	{
		// checking if this and the ac obj are the same
		return std::memcmp((void*)this, (void*)(&ac), sizeof(attributeCombo)) == 0;
	};

	// overwriting the less than operator
	bool operator<(const attributeCombo ac) const
	{
		// checking if this and the ac obj are the same
		return std::memcmp((void*)this, (void*)(&ac), sizeof(attributeCombo)) > 0;
	};

	glm::vec3 vertex;
	glm::vec2 uv;
	glm::vec3 normal;
};

// checking if the combo exists in the map
int exists(
	 std::map<attributeCombo, int>& map,
	 attributeCombo& combo
)
{
	// try to find the combo in the map
	std::map<attributeCombo, int>::iterator i = map.find(combo);

	// if it is found we return the second value, which is the existing index
	// otherwise returning a -1 for it not being in there
	if (map.find(combo) != map.end())
		return i->second;
	else
		return -1;
}

void vboIndex(
	const std::vector<ObjInfo>& objInfo,
	std::vector<ObjInfo>& out_objInfo
)
{
	// loop through every element
	for (int i = 0; i < objInfo.size(); ++i)
	{
		// trying to use the map function to speed up the lookup
		std::map<attributeCombo, int> map;
		int idx = 0;
		ObjInfo oi;

		int found = -1;
		for (int j = 0; j < objInfo[i].normals.size(); ++j)
		{
			// checking if vertex, texture coord and normal combo is already in the ObjInfo
			attributeCombo ac = attributeCombo(objInfo[i].vertices[j], objInfo[i].uvs[j], objInfo[i].normals[j]);
			int index = exists(map, ac);

			// not already on there
			if (index < 0)
			{
				oi.vertices.push_back(objInfo[i].vertices[j]);
				oi.uvs.push_back(objInfo[i].uvs[j]);
				oi.normals.push_back(objInfo[i].normals[j]);
				oi.indices.push_back(idx);
				map[ac] = idx++;
			}
			else
			{
				// it was alredy there so just adding the index
				oi.indices.push_back(index);
			}
		}
		// need the texture index
		oi.txIdx = objInfo[i].txIdx;

		oi.center();

		out_objInfo.push_back(oi);
	}
}