#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>

#include <glm/glm.hpp>

#include "ModelLoader.hpp"

bool loadObj(
	const char * path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
)
{
	printf("Loading .obj file %s...\n",	path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	// open file
	std::ifstream inputFile(path, std::ios::in);

	if (!inputFile.is_open()) {
		printf("Impossible to open the file %s! Are you in the right path ?\n", path);
		getchar();
		return false;
	}

	char line[256];

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
		}
	}

	inputFile.close();

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) 
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}

	return 0;
}