#ifndef OBJINFO_H
#define OBJINFO_H

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>

class ObjInfo
{
public:
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

	float size()
	{
		if (_size != 0.0f)
			return _size;

		glm::vec3 c = center();

		float max_length = 0.0f;

		for (int i = 0; i < vertices.size(); i++)
		{
			glm::vec3 t = vertices[i] - c;

			float test2 = t.x * t.x + t.y * t.y + t.z * t.z;

			if (test2 > max_length)
				max_length = test2;
		}

		_size = sqrt(max_length);
	}

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	int txIdx;

private:
	glm::vec3 _center = glm::vec3(INFINITY);
	float _size = 0.0f;
};

#endif // !OBJINFO_H
