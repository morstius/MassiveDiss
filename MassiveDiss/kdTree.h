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
	bool isLeaf;
	bool splitX;
	int idx;
	ObjInfo objinfo;
	glm::vec3 point;
	struct kdNode *left, *right;

	std::chrono::duration<double> time_sorting;
	std::chrono::duration<double> time_creatingLeafNode;
	std::chrono::duration<double> time_creatingIntNode;
};

std::vector<ObjInfo> sort(std::vector<ObjInfo>, bool);

kdNode* kdTreeConstruct(std::vector<ObjInfo>, int, int);

ObjInfo XSplit(std::vector<ObjInfo>, std::vector<ObjInfo>&, std::vector<ObjInfo>&);

ObjInfo ZSplit(std::vector<ObjInfo>, std::vector<ObjInfo>&, std::vector<ObjInfo>&);

kdNode* CreateInteriorNode(ObjInfo, kdNode*, kdNode*, bool);

kdNode* CreateLeafNode(std::vector<ObjInfo>);

#endif // !KDTREE_H
