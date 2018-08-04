// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>
#include <algorithm>

#include "common/types/ObjInfo.h"

#include "kdTree.h"

bool zCompare(const ObjInfo &a, const ObjInfo &b) { return (a._center.z < b._center.z); }
bool xCompare(const ObjInfo &a, const ObjInfo &b) { return (a._center.x < b._center.x); }

kdNode* kdTreeConstruct(std::vector<ObjInfo>& objectSet, int depth, int size)
{
	if (size <= 1)
	{
		// creating a leaf node
		return CreateLeafNode(objectSet);
	}
	else
	{
		std::vector<ObjInfo> leftHalf, rightHalf;
		ObjInfo objInfo;

		// splitting either on the x or the z axis
		if (depth % 2 == 0) // even
		{
			std::sort(objectSet.begin(), objectSet.end(), xCompare);
			XSplit(objectSet, leftHalf, rightHalf);
		}
		else
		{
			std::sort(objectSet.begin(), objectSet.end(), zCompare);
			ZSplit(objectSet, leftHalf, rightHalf);
		}

		// recursively create the left and right sides of the tree
		kdNode *leftNode = nullptr, *rightNode = nullptr;
		if((int)leftHalf.size() > 0)
			 leftNode = kdTreeConstruct(leftHalf, depth + 1, (int)leftHalf.size());
		if((int)rightHalf.size() > 0)
			rightNode = kdTreeConstruct(rightHalf, depth + 1, (int)rightHalf.size());

		return CreateInteriorNode(leftNode, rightNode, depth % 2 == 0 ? kdNode::axisSplit::xaxis : kdNode::axisSplit::zaxis);
	}
}

void XSplit(const std::vector<ObjInfo>& objectSet, std::vector<ObjInfo>& leftHalf, std::vector<ObjInfo>& rightHalf)
{
	int half = (int)objectSet.size() / 2;

	for (int i = 0; i < objectSet.size(); i++)
	{
		if (i < half)
		{
			leftHalf.push_back(objectSet[i]);
		}
		if (i >= half)
		{
			rightHalf.push_back(objectSet[i]);
		}
	}
}

void ZSplit(const std::vector<ObjInfo>& objectSet, std::vector<ObjInfo>& leftHalf, std::vector<ObjInfo>& rightHalf)
{
	int half = (int)objectSet.size() / 2;

	for (int i = 0; i < objectSet.size(); i++)
	{
		if (i < half)
		{
			leftHalf.push_back(objectSet[i]);
		}
		if (i >= half)
		{
			rightHalf.push_back(objectSet[i]);
		}
	}
}

kdNode* CreateInteriorNode(kdNode* leftHalf, kdNode* rightHalf, kdNode::axisSplit split)
{
	kdNode* kd = new kdNode();

	kd->isLeaf = false;
	kd->axis = split;

	kd->left = leftHalf;
	kd->right = rightHalf;

	return kd;
}

kdNode* CreateLeafNode(std::vector<ObjInfo> objectSet)
{
	kdNode* kd = new kdNode();

	kd->isLeaf = true;
	kd->objInfo = objectSet;

	return kd;
}