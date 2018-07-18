#ifndef OBJINFO_H
#define OBJINFO_H

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>

class ObjInfo
{
public:

	// for sorting the x value
	bool operator < (const ObjInfo& oi) const
	{
		return (_center.x < oi._center.x);
	}

	// for sorting the y value
	bool operator > (const ObjInfo& oi) const
	{
		return (_center.y < oi._center.y);
	}

	glm::vec3 center()
	{
		if (_center.x == INFINITY)
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

	float size(float& minX, float& maxX, float& minZ, float& maxZ)
	{
		if (_minX == _maxX)
		{
			for (int i = 0; i < vertices.size(); i++)
			{
				if (vertices[i].x < _minX)
					_minX = vertices[i].x;

				if (vertices[i].x > _maxX)
					_maxX = vertices[i].x;

				if (vertices[i].z < _minZ)
					_minZ = vertices[i].z;

				if (vertices[i].z > _maxZ)
					_maxZ = vertices[i].z;
			}
		}

		minX = _minX;
		maxX = _maxX;

		minZ = _minZ;
		maxZ = _maxZ;
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	int txIdx;

private:
	glm::vec3 _center = glm::vec3(INFINITY);
	float _minX = 0;
	float _maxX = 0;
	float _minZ = 0;
	float _maxZ = 0;
};

#endif // !OBJINFO_H
