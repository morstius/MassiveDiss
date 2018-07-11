#ifndef MTLOBJ_H
#define MTLOBJ_H

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

class MtlObj
{
public:
	MtlObj(){};
	MtlObj(char* name) : newmtl(name) {};

	char* newmtl;
	char* map_Kd;
	char* map_Ks;
	char* map_Bump;
	glm::vec3 Kd;
	glm::vec3 Ka;
	glm::vec3 Ks;
	int Ns;
	int d;
	int illum;
};

#endif // !MTLOBJ_H
