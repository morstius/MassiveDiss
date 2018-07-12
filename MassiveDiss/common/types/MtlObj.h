#ifndef MTLOBJ_H
#define MTLOBJ_H

#include "GL/glew.h"

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <string>

class MtlObj
{
public:
	MtlObj() : newmtl("none") {};
	MtlObj(std::string _newmtl, std::string _map_Kd, bool _hasText)
		: newmtl(_newmtl), map_Kd(_map_Kd), hasTexture(_hasText) {};

	std::string newmtl;
	std::string map_Kd;
	char* map_Ks;
	char* map_Bump;
	glm::vec3 Kd;
	glm::vec3 Ka;
	glm::vec3 Ks;
	int Ns;
	int d;
	int illum;
	bool hasTexture = false;
	GLuint textNbr;
};

#endif // !MTLOBJ_H
