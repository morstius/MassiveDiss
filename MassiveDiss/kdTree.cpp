// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <vector>
#include <algorithm>

#include "common/types/ObjInfo.h"

#include "kdTree.h"

// for profiling
#include <ctime>
#include <chrono>

std::chrono::duration<double> time_sorting;
std::chrono::duration<double> time_creatingLeafNode;
std::chrono::duration<double> time_creatingIntNode;

kdNode* kdTreeConstruct(std::vector<ObjInfo> objectSet, int depth, int size)
{
	if (size == 1)
	{
		return CreateLeafNode(objectSet);
	}
	else
	{
		std::vector<ObjInfo> leftHalf, rightHalf;
		ObjInfo objInfo;

		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		if (depth % 2 == 0) // even
		{
			objInfo = XSplit(objectSet, leftHalf, rightHalf);
		}
		else
		{
			objInfo = ZSplit(objectSet, leftHalf, rightHalf);
		}
		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
		time_sorting += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

		kdNode *leftNode = nullptr, *rightNode = nullptr;
		if((int)leftHalf.size() > 0)
			 leftNode = kdTreeConstruct(leftHalf, depth + 1, (int)leftHalf.size());
		if((int)rightHalf.size() > 0)
			rightNode = kdTreeConstruct(rightHalf, depth + 1, (int)rightHalf.size());

		return CreateInteriorNode(objInfo, leftNode, rightNode, depth % 2);
	}
}

std::vector<ObjInfo> sort(std::vector<ObjInfo> objInfo, bool sortX)
{
	if (objInfo.size() == 1)
		return objInfo;

	int half = (int)objInfo.size() / 2;

	std::vector<ObjInfo> firstHalf, secondHalf;
	for (int i = 0; i < objInfo.size(); i++)
	{
		if (i < half)
			firstHalf.push_back(objInfo[i]);
		else
			secondHalf.push_back(objInfo[i]);
	}

	firstHalf = sort(firstHalf, sortX);
	secondHalf = sort(secondHalf, sortX);

	std::vector<ObjInfo> res;
	
	bool stop = false;
	int i = 0, j = 0;
	while (!stop)
	{
		if (i < firstHalf.size() && j < secondHalf.size())
		{
			if (sortX)
			{
				if (firstHalf[i].center().x < secondHalf[j].center().x)
				{
					res.push_back(firstHalf[i]);
					i++;
				}
				else
				{
					res.push_back(secondHalf[j]);
					j++;
				}
			}
			else
			{
				if (firstHalf[i].center().z < secondHalf[j].center().z)
				{
					res.push_back(firstHalf[i]);
					i++;
				}
				else
				{
					res.push_back(secondHalf[j]);
					j++;
				}
			}
		}
		else if (i >= firstHalf.size())
		{
			res.push_back(secondHalf[j]);
			j++;
		}
		else if (j >= secondHalf.size())
		{
			res.push_back(firstHalf[i]);
			i++;
		}

		if (i >= firstHalf.size() && j >= secondHalf.size())
			stop = true;
	}

	return res;	
}

bool xCompare(const ObjInfo &a, const ObjInfo &b) { return (a._center.x < b._center.x); }


ObjInfo XSplit(std::vector<ObjInfo> objectSet, std::vector<ObjInfo>& leftHalf, std::vector<ObjInfo>& rightHalf)
{
	//objectSet = sort(objectSet, true);

	std::sort(objectSet.begin(), objectSet.end(), xCompare);

	int half = (int)objectSet.size() / 2;

	for (int i = 0; i < objectSet.size(); i++)
	{
		if (i < half)
		{
			leftHalf.push_back(objectSet[i]);
		}
		if (i > half)
		{
			rightHalf.push_back(objectSet[i]);
		}
	}

	return objectSet[half];
}

bool zCompare(const ObjInfo &a, const ObjInfo &b) { return (a._center.z < b._center.z); }

ObjInfo ZSplit(std::vector<ObjInfo> objectSet, std::vector<ObjInfo>& leftHalf, std::vector<ObjInfo>& rightHalf)
{
	//objectSet = sort(objectSet, false);

	std::sort(objectSet.begin(), objectSet.end(), zCompare);

	int half = (int)objectSet.size() / 2;

	for (int i = 0; i < objectSet.size(); i++)
	{
		if (i < half)
		{
			leftHalf.push_back(objectSet[i]);
		}
		if (i > half)
		{
			rightHalf.push_back(objectSet[i]);
		}
	}

	return objectSet[half];
}

kdNode* CreateInteriorNode(ObjInfo objInfo, kdNode* leftHalf, kdNode* rightHalf, bool splitX)
{
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	kdNode* kd = new kdNode();

	kd->isLeaf = false;
	kd->splitX = splitX;

	kd->objinfo = objInfo;

	kd->left = leftHalf;
	kd->right = rightHalf;

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	time_creatingIntNode += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	
	
	kd->time_sorting = time_sorting;
	kd->time_creatingIntNode = time_creatingIntNode;
	kd->time_creatingLeafNode = time_creatingLeafNode;

	return kd;
}

kdNode* CreateLeafNode(std::vector<ObjInfo> objectSet)
{
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	kdNode* kd = new kdNode();

	kd->isLeaf = true;

	kd->objinfo = objectSet[0];

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	time_creatingLeafNode += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

	kd->time_sorting = time_sorting;
	kd->time_creatingIntNode = time_creatingIntNode;
	kd->time_creatingLeafNode = time_creatingLeafNode;

	return kd;
}