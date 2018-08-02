// Include GLM
#include <glm/glm.hpp>

#include <array>
#include <math.h>

class Frustum
{
public:
	std::array<glm::vec4, 6> planes;

	void update(glm::mat4 matrix)
	{
		glm::vec4 matX = glm::vec4(matrix[0].x, matrix[1].x, matrix[2].x, matrix[3].x);
		glm::vec4 matY = glm::vec4(matrix[0].y, matrix[1].y, matrix[2].y, matrix[3].y);
		glm::vec4 matZ = glm::vec4(matrix[0].z, matrix[1].z, matrix[2].z, matrix[3].z);
		glm::vec4 matW = glm::vec4(matrix[0].w, matrix[1].w, matrix[2].w, matrix[3].w);
		
		//left plane
		planes[0] = matW + matX;

		//right plane
		planes[1] = matW - matX;
		
		//bottom plane
		planes[2] = matW + matY;

		//top plane
		planes[3] = matW - matY;

		//back plane
		planes[4] = matW + matZ;
		
		//front plane
		planes[5] = matW - matZ;

		//normalize
		for (auto i = 0; i < planes.size(); i++)
		{
			planes[i] = glm::normalize(planes[i]);
		}
	}

	bool checkSphere(glm::vec3 pos, float radius)
	{
		for (auto i = 0; i < planes.size(); i++)
		{
			// check if bounding sphere for object is inside the frustum
			if ((planes[i].x * pos.x) + (planes[i].y * pos.y) + (planes[i].z * pos.z) + planes[i].w <= -radius)
			{
				return false;
			}
		}
		return true;
	}
};