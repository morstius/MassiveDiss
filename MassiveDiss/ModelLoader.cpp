#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>

#include <glm/glm.hpp>

#include "ModelLoader.hpp"
#include "common/types/MtlObj.h"
#include "common/types/ObjInfo.h"
#include "common/types/IdxInfo.h"

bool loadObj(
	const char * path,
	std::vector<ObjInfo>& out_objInfo,
	std::vector<MtlObj>& textureLib
)
{
	printf("Loading .obj file %s...\n",	path);

	std::vector<IdxInfo> idxInfo;
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	std::vector<int> temp_texInd;

	// open file
	std::ifstream inputFile(path, std::ios::in);

	if (!inputFile.is_open()) {
		printf("Impossible to open the file %s! Are you in the right path ?\n", path);
		getchar();
		return false;
	}

	char line[256];
	char curText[50];

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
						temp_vertices.push_back(vec);
						break;
					}
					case 'n':
					{
						// dummy for values
						glm::vec3 vec;

						sscanf(line, "%s %f %f %f", dummy, &(vec.x), &(vec.y), &(vec.z));
						temp_normals.push_back(vec);
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
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = sscanf(line, "%s %d/%d/%d %d/%d/%d %d/%d/%d\n", dummy, &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				// checking that the format was correct
				if (matches != 10) {
					printf("File can't be read by our simple parser! Try exporting with other options\n");
					inputFile.close();
					return false;
				}

				// set vertex indices
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				// set uv indices
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				// set normal indices
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);

				break;
			}
			case 'o':
			{
				char objName[256];
				sscanf(line, "%s %s", dummy, &objName);
				printf("%s\n", objName);

				if (vertexIndices.size() != 0)
				{
					IdxInfo ii = IdxInfo();
					ii.normalIndices = normalIndices;
					ii.uvIndices = uvIndices;
					ii.vertexIndices = vertexIndices;
					ii.textureIndex = findTexIdx(curText, textureLib);
					
					idxInfo.push_back(ii);

					vertexIndices.clear();
					normalIndices.clear();
					uvIndices.clear();
				}

				break;
			}
			case 'm':
			{
				char objName[256];
				sscanf(line, "%s %s", &dummy, &objName);
				if (strcmp(dummy, "mtllib") == 0)
				{
					printf("Loading material lib...\n");
					populateMtlLib(objName, textureLib);
				}
			}
			case 'u':
			{
				char objName[256];
				sscanf(line, "%s %s", &dummy, &objName);

				if (strcmp(dummy, "usemtl") == 0)
				{
					strcpy(curText, objName);
				}
			}
		}
	}

	IdxInfo ii = IdxInfo();
	ii.normalIndices = normalIndices;
	ii.uvIndices = uvIndices;
	ii.vertexIndices = vertexIndices;
	ii.textureIndex = findTexIdx(curText, textureLib);

	idxInfo.push_back(ii);

	inputFile.close();

	// For each vertex of each triangle
	for (unsigned int j = 0; j < idxInfo.size(); j++)
	{
		ObjInfo oi = ObjInfo();
		for (unsigned int i = 0; i < idxInfo[j].vertexIndices.size(); i++)
		{
			// Get the indices of its attributes
			unsigned int vertexIndex = idxInfo[j].vertexIndices[i];
			unsigned int uvIndex = idxInfo[j].uvIndices[i];
			unsigned int normalIndex = idxInfo[j].normalIndices[i];

			// Get the attributes thanks to the index
			glm::vec3 vertex = temp_vertices[vertexIndex - 1];
			glm::vec2 uv = temp_uvs[uvIndex - 1];
			glm::vec3 normal = temp_normals[normalIndex - 1];

			// Put the attributes in buffers
			oi.vertices.push_back(vertex);
			oi.uvs.push_back(uv);
			oi.normals.push_back(normal);
			oi.txIdx = idxInfo[j].textureIndex;
		}
		out_objInfo.push_back(oi);
	}

	return 0;
}

bool populateMtlLib(
	const char * filepath,
	std::vector<MtlObj>& textureLib
)
{
	const char * base = "models/";
	char* path;
	path = (char*)malloc(strlen(base) + strlen(filepath) + 1);
	strcpy(path, base);
	strcat(path, filepath);


	// open file
	std::ifstream inputFile(path, std::ios::in);

	if (!inputFile.is_open()) {
		printf("Impossible to open the file %s! Are you in the right path ?\n", path);
		getchar();
		return false;
	}

	MtlObj mtlObj = MtlObj();

	char line[256];

	while (inputFile.getline(line, 256))
	{
		// a dummy for the first letters
		char dummy[10];

		char objName[50];
		sscanf(line, "%s %s", &dummy, &objName);

		if (strcmp(dummy, "newmtl") == 0)
		{
			if (mtlObj.newmtl != NULL)
				textureLib.push_back(mtlObj);

			mtlObj = MtlObj(objName);
		}

		if (strcmp(dummy, "map_Kd") == 0)
		{
			const char * bass = "textures/";
			char* textPath;
			textPath = (char*)malloc(strlen(bass) + strlen(objName) + 1);
			strcpy(textPath, bass);
			strcat(textPath, objName);

			mtlObj.map_Kd = textPath;
		}
	}

	textureLib.push_back(mtlObj);

	return 0;
}

int findTexIdx(
	const char * curTxt,
	std::vector<MtlObj> txtLib
)
{
	int idx = 0;
	for (std::vector<MtlObj>::iterator it = txtLib.begin(); it != txtLib.end(); ++it, ++idx) {
		if (strcmp(curTxt, (*it).newmtl))
			return idx;
	}

	printf("Texture not found!!!\n");

	return idx;
}