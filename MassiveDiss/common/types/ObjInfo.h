#ifndef OBJINFO_H
#define OBJINFO_H

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>

class ObjInfo
{
public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	int txIdx;
};

#endif // !OBJINFO_H
