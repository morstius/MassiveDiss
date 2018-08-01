#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "vboindexer.h"

#include <string.h> // for memcmp

#include "types/ObjInfo.h"


struct PackedVertex{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	bool operator<(const PackedVertex that) const{
		return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
	};
};

bool getSimilarVertexIndex_fast( 
	PackedVertex & packed, 
	std::map<PackedVertex,unsigned int> & VertexToOutIndex,
	unsigned int & result
){
	std::map<PackedVertex,unsigned int>::iterator it = VertexToOutIndex.find(packed);
	if ( it == VertexToOutIndex.end() ){
		return false;
	}else{
		result = it->second;
		return true;
	}
}

void indexVBO(
	std::vector<ObjInfo> objInfo,
	std::vector<ObjInfo>& out_objInfo
){
	std::map<PackedVertex,unsigned int> VertexToOutIndex;

	// For each input vertex
	for (unsigned int j = 0; j < objInfo.size(); j++)
	{
		ObjInfo oi = ObjInfo();
		for (unsigned long i = 0; i < objInfo[j].vertices.size(); i++) {

			PackedVertex packed = { objInfo[j].vertices[i], objInfo[j].uvs[i], objInfo[j].normals[i] };

			// Try to find a similar vertex in out_objInfo
			unsigned int index;
			bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

			if (found) 
			{ // A similar vertex is already in the VBO, use it instead !
				oi.indices.push_back(index);
			}
			else 
			{ // If not, it needs to be added in the output data.
				oi.vertices.push_back(objInfo[j].vertices[i]);
				oi.uvs.push_back(objInfo[j].uvs[i]);
				oi.normals.push_back(objInfo[j].normals[i]);
				unsigned int newindex = (unsigned int)oi.vertices.size() - 1;
				oi.indices.push_back(newindex);
				VertexToOutIndex[packed] = newindex;
			}

			oi.txIdx = objInfo[j].txIdx;
		}
		out_objInfo.push_back(oi);
	}
}
