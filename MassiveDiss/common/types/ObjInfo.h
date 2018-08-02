#ifndef OBJINFO_H
#define OBJINFO_H

#include "GL/glew.h"

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>

class ObjInfo
{
public:
	ObjInfo()
	{
		//Init
		//glCreateVertexArrays(1, &boundingBoxArray);
		this->center();
	}
	~ObjInfo()
	{
	}

	glm::vec3 center()
	{
		// calculate the position of the center
		if (_center.x == INFINITY || _center == glm::vec3(0) || isnan(_center.x))
		{
			glm::vec3 sum = glm::vec3(0.0f);
			for (int i = 0; i < vertices.size(); i++)
			{
				sum += vertices[i];
			}

			sum.x /= vertices.size();
			sum.y /= vertices.size();
			sum.z /= vertices.size();

			_center = sum;
		}

		return _center;
	}

	float size()
	{
		if (_size != 0.0f)
			return _size;

		glm::vec3 c = center();

		float max_length = 0.0f;

		// calculating the maximum distance a vertice is away from the center
		for (int i = 0; i < vertices.size(); i++)
		{
			glm::vec3 t = vertices[i] - c;

			float test2 = t.x * t.x + t.y * t.y + t.z * t.z;

			if (test2 > max_length)
				max_length = test2;
		}

		_size = sqrt(max_length);
		return _size;
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	int txIdx;

	std::vector<glm::vec3> boundingVert;

	glm::vec3 min;
	glm::vec3 max;

	GLuint query;
	bool queryInProgress = false;
	bool occluded = false;

	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	GLuint elembuffer;

	glm::vec3 _center;

private:
	float _size = 0.0f;
};

#endif // !OBJINFO_H
