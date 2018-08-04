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
	enum axisSplit { xaxis = 0, yaxis = 1, zaxis = 2 };

	glm::vec3 split;
	axisSplit axis;

	kdNode *left, *right;

	bool isLeaf;
	std::vector<ObjInfo> objInfo;
};

kdNode* kdTreeConstruct(std::vector<ObjInfo>, int, int);

void XSplit(const std::vector<ObjInfo>&, std::vector<ObjInfo>&, std::vector<ObjInfo>&);

void ZSplit(const std::vector<ObjInfo>&, std::vector<ObjInfo>&, std::vector<ObjInfo>&);

kdNode* CreateInteriorNode(kdNode*, kdNode*, kdNode::axisSplit);

kdNode* CreateLeafNode(std::vector<ObjInfo>);

#endif // !KDTREE_H
