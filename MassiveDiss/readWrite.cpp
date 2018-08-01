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

void writeFile(const std::vector<ObjInfo>& objInfo, const std::vector<MtlObj>& textureLib, std::string name)
{
	FILE* file;

	// open file
	file = fopen(std::string(name + ".objs").c_str(), "w");

	fprintf(file, "# preprocessed .obj file\n");

	// write each object in vector
	for (int i = 0; i < objInfo.size(); ++i)
	{
		fprintf(file, "o obj %d\n", i);
		fprintf(file, "t %s\n", textureLib[objInfo[i].txIdx].map_Kd.c_str());
		for (int j = 0; j < objInfo[i].vertices.size(); ++j)
		{
			fprintf(file, "v %f %f %f\n", objInfo[i].vertices[j].x, objInfo[i].vertices[j].y, objInfo[i].vertices[j].z);
			fprintf(file, "vt %f %f\n", objInfo[i].uvs[j].x, objInfo[i].uvs[j].y);
			fprintf(file, "vn %f %f %f\n", objInfo[i].normals[j].x, objInfo[i].normals[j].y, objInfo[i].normals[j].z);
		}

		for (int j = 0; j < objInfo[i].indices.size(); ++j)
		{
			fprintf(file, "i %d\n", objInfo[i].indices[j]);
		}
	}

	// close file
	fclose(file);
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
		}
	}

	objInfo.push_back(oi);

	// close file
	file.close();
}