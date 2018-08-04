// Include GLM
#include <glm/glm.hpp>

#include <array>
#include <math.h>

class Frustum
{
public:
	std::array<glm::vec4, 6> frustPlanes;

	void update(glm::mat4 matrix)
	{
		glm::vec4 matX = glm::vec4(matrix[0].x, matrix[1].x, matrix[2].x, matrix[3].x);
		glm::vec4 matY = glm::vec4(matrix[0].y, matrix[1].y, matrix[2].y, matrix[3].y);
		glm::vec4 matZ = glm::vec4(matrix[0].z, matrix[1].z, matrix[2].z, matrix[3].z);
		glm::vec4 matW = glm::vec4(matrix[0].w, matrix[1].w, matrix[2].w, matrix[3].w);
		
		//left plane
		frustPlanes[0] = matW + matX;
		frustPlanes[0] = glm::normalize(frustPlanes[0]);

		//right plane
		frustPlanes[1] = matW - matX;
		frustPlanes[1] = glm::normalize(frustPlanes[1]);
		
		//bottom plane
		frustPlanes[2] = matW + matY;
		frustPlanes[2] = glm::normalize(frustPlanes[2]);

		//top plane
		frustPlanes[3] = matW - matY;
		frustPlanes[3] = glm::normalize(frustPlanes[3]);

		//back plane
		frustPlanes[4] = matW + matZ;
		frustPlanes[4] = glm::normalize(frustPlanes[4]);
		
		//front plane
		frustPlanes[5] = matW - matZ;
		frustPlanes[5] = glm::normalize(frustPlanes[5]);
	}

	bool checkSphere(glm::vec3 pos, float radius)
	{
		for (auto i = 0; i < frustPlanes.size(); i++)
		{
			// check if bounding sphere for object is inside the frustum
			if ((frustPlanes[i].x * pos.x) + (frustPlanes[i].y * pos.y) + (frustPlanes[i].z * pos.z) + frustPlanes[i].w <= -radius)
			{
				return false;
			}
		}
		return true;
	}
};