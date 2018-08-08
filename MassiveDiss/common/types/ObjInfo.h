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

	void createBoundingBox()
	{
		if (this->vertices.size() <= 0)
			return;

		float minX, maxX, minY, maxY, minZ, maxZ;

		minX = this->vertices[0].x;
		maxX = this->vertices[0].x;

		minY = this->vertices[0].y;
		maxY = this->vertices[0].y;

		minZ = this->vertices[0].z;
		maxZ = this->vertices[0].z;

		// finding the max and min values for x,y and z
		for (int j = 0; j < this->vertices.size(); ++j)
		{
			if (this->vertices[j].x < minX)
				minX = this->vertices[j].x;
			if (this->vertices[j].x > maxX)
				maxX = this->vertices[j].x;

			if (this->vertices[j].y < minY)
				minY = this->vertices[j].y;
			if (this->vertices[j].y > maxY)
				maxY = this->vertices[j].y;

			if (this->vertices[j].z < minZ)
				minZ = this->vertices[j].z;
			if (this->vertices[j].z > maxZ)
				maxZ = this->vertices[j].z;
		}

		// creating the 8 corners of the bounding box
		glm::vec3 up_left_deep = glm::vec3(minX, maxY, maxZ);
		glm::vec3 up_left_near = glm::vec3(minX, maxY, minZ);

		glm::vec3 up_right_deep = glm::vec3(maxX, maxY, maxZ);
		glm::vec3 up_right_near = glm::vec3(maxX, maxY, minZ);

		glm::vec3 down_left_deep = glm::vec3(minX, minY, maxZ);
		glm::vec3 down_left_near = glm::vec3(minX, minY, minZ);

		glm::vec3 down_right_deep = glm::vec3(maxX, minY, maxZ);
		glm::vec3 down_right_near = glm::vec3(maxX, minY, minZ);

		// deep
		this->boundingVert.push_back(up_left_deep);
		this->boundingVert.push_back(down_left_deep);
		this->boundingVert.push_back(up_right_deep);

		this->boundingVert.push_back(down_right_deep);
		this->boundingVert.push_back(up_right_deep);
		this->boundingVert.push_back(down_left_deep);

		// near
		this->boundingVert.push_back(up_left_near);
		this->boundingVert.push_back(up_right_near);
		this->boundingVert.push_back(down_left_near);

		this->boundingVert.push_back(down_right_near);
		this->boundingVert.push_back(down_left_near);
		this->boundingVert.push_back(up_right_near);

		// left
		this->boundingVert.push_back(up_left_deep);
		this->boundingVert.push_back(up_left_near);
		this->boundingVert.push_back(down_left_near);

		this->boundingVert.push_back(down_left_near);
		this->boundingVert.push_back(down_left_deep);
		this->boundingVert.push_back(up_left_deep);

		// right
		this->boundingVert.push_back(up_right_deep);
		this->boundingVert.push_back(down_right_near);
		this->boundingVert.push_back(up_right_near);

		this->boundingVert.push_back(down_right_near);
		this->boundingVert.push_back(up_right_deep);
		this->boundingVert.push_back(down_right_deep);

		// top
		this->boundingVert.push_back(up_right_deep);
		this->boundingVert.push_back(up_left_near);
		this->boundingVert.push_back(up_left_deep);

		this->boundingVert.push_back(up_right_deep);
		this->boundingVert.push_back(up_right_near);
		this->boundingVert.push_back(up_left_near);

		// bottom
		this->boundingVert.push_back(down_right_deep);
		this->boundingVert.push_back(down_left_deep);
		this->boundingVert.push_back(down_left_near);

		this->boundingVert.push_back(down_right_deep);
		this->boundingVert.push_back(down_left_near);
		this->boundingVert.push_back(down_right_near);
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

		// max length is the length squared, so need to take the root
		_size = sqrt(max_length);
		return _size;
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	int txIdx;

	bool useForOcclusion;
	std::vector<glm::vec3> boundingVert;

	glm::vec3 min;
	glm::vec3 max;

	GLuint queryID;
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
