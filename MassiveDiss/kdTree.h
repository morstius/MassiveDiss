#ifndef KDTREE_H
#define KDTREE_H

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>

#include "common/types/ObjInfo.h"

// for profiling
#include <ctime>
#include <chrono>

struct kdNode 
{
	// made this an enum for readability
	enum axisSplit { xaxis = 0, yaxis = 1, zaxis = 2 };

	float split;
	axisSplit axis;

	kdNode *left, *right;

	bool isLeaf;
	std::vector<ObjInfo> objInfo;
};

kdNode* kdTreeConstruct(std::vector<ObjInfo>&, int, int);

#endif // !KDTREE_H
