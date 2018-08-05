// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>
#include <algorithm>

#include "common/types/ObjInfo.h"

#include "kdTree.h"

bool zCompare(const ObjInfo &a, const ObjInfo &b) { return (a._center.z < b._center.z); }
bool xCompare(const ObjInfo &a, const ObjInfo &b) { return (a._center.x < b._center.x); }

void XSplit(const std::vector<ObjInfo>& objectSet, std::vector<ObjInfo>& leftHalf, std::vector<ObjInfo>& rightHalf, float& point)
{
	int half = (int)objectSet.size() / 2;

	// updating where the split was through the median
	if (objectSet.size() % 2 == 0)
	{
		point = (objectSet[half]._center.x + objectSet[half - 1]._center.x) / 2.0f;
	}
	else
	{
		point = objectSet[half]._center.x;
	}

	std::vector<ObjInfo>::const_iterator mid = objectSet.begin() + half;
	leftHalf = std::vector<ObjInfo>(objectSet.begin(), mid);
	rightHalf = std::vector<ObjInfo>(mid, objectSet.end());
}

void ZSplit(const std::vector<ObjInfo>& objectSet, std::vector<ObjInfo>& leftHalf, std::vector<ObjInfo>& rightHalf, float& point)
{
	int half = (int)objectSet.size() / 2;

	// updating where the split was through the median
	if (objectSet.size() % 2 == 0)
	{
		point = (objectSet[half]._center.z + objectSet[half - 1]._center.z) / 2.0f;
	}
	else
	{
		point = objectSet[half]._center.z;
	}

	std::vector<ObjInfo>::const_iterator mid = objectSet.begin() + half;
	leftHalf = std::vector<ObjInfo>(objectSet.begin(), mid);
	rightHalf = std::vector<ObjInfo>(mid, objectSet.end());
}

kdNode* CreateInteriorNode(kdNode* leftHalf, kdNode* rightHalf, kdNode::axisSplit split, float point)
{
	kdNode* kd = new kdNode();

	kd->isLeaf = false;
	kd->axis = split;

	kd->split = point;

	kd->left = leftHalf;
	kd->right = rightHalf;

	return kd;
}

kdNode* CreateLeafNode(const std::vector<ObjInfo>& objectSet)
{
	kdNode* kd = new kdNode();

	kd->isLeaf = true;
	kd->objInfo = objectSet;

	return kd;
}

kdNode* kdTreeConstruct(std::vector<ObjInfo>& objectSet, int depth, int size)
{
	// the maximum amount of objects in the leafs can be changed,
	// some papers mentions having possibly max 3 per leaf
	if (size <= 1)
	{
		// creating a leaf node
		return CreateLeafNode(objectSet);
	}
	else
	{
		std::vector<ObjInfo> leftHalf, rightHalf;
		ObjInfo objInfo;
		float point;

		// splitting either on the x or the z axis
		if (depth % 2 == 0) // even
		{
			// sort first and then split
			std::sort(objectSet.begin(), objectSet.end(), xCompare);
			XSplit(objectSet, leftHalf, rightHalf, point);
		}
		else
		{
			// sort first and then split
			std::sort(objectSet.begin(), objectSet.end(), zCompare);
			ZSplit(objectSet, leftHalf, rightHalf, point);
		}

		// recursively create the left and right sides of the tree
		kdNode *leftNode = nullptr, *rightNode = nullptr;

		// recursivly create each branch of the tree
		leftNode = kdTreeConstruct(leftHalf, depth + 1, (int)leftHalf.size());
		rightNode = kdTreeConstruct(rightHalf, depth + 1, (int)rightHalf.size());

		return CreateInteriorNode(leftNode, rightNode, depth % 2 == 0 ? kdNode::axisSplit::xaxis : kdNode::axisSplit::zaxis, point);
	}
}