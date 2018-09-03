#ifndef OCCLUSIONCHECK_H
#define OCCLUSIONCHECK_H

#include "GL/glew.h"

// Include GLFW
#include "GLFW/glfw3.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "kdTree.h"

#include "common/frustum.h"

#include "common/types/ObjInfo.h"
#include "common/types/MtlObj.h"

#include <unordered_map>

void makeOcclusionCheck(
	GLFWwindow*,
	kdNode*,
	Frustum&,
	int&,
	int&,
	int&,
	int&,
	glm::vec3,
	glm::mat4&,
	std::vector<MtlObj>&,
	std::vector<GLuint>&,
	GLuint BoundingBoxProgramID,
	GLuint BoundingMatrixID,
	GLuint ProgramID,
	GLuint TextureID,
	GLuint MatrixID,
	GLuint ViewMatrixID
);

void randomLocation(
	kdNode* tree,
	glm::mat4& MVP,
	float& minX,
	float& maxX,
	float& minY,
	float& maxY,
	float& minZ,
	float& maxZ
);

void checkOcclusion(
	kdNode*, 
	std::unordered_map<std::string, int>&
);

void printOcclusion(
	kdNode*,
	std::unordered_map<std::string, int>
);

#endif // !OCCLUSIONCHECK_H
