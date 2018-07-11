#ifndef IDXINFO_H
#define IDXINFO_H

#include <vector>

class IdxInfo
{
public:
	std::vector<unsigned int> vertexIndices;
	std::vector<unsigned int> uvIndices;
	std::vector<unsigned int> normalIndices;
	int textureIndex;
};

#endif // !IDXINFO_H
