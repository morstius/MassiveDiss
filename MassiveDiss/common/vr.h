#ifndef VR_H
#define VR_H

#include "GL/glew.h"
#include <openvr/headers/openvr.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <string>
#include <vector>

#include "types/ObjInfo.h"
#include "types/MtlObj.h"

struct FramebufferDesc
{
	GLuint m_nDepthBufferId;
	GLuint m_nRenderTextureId;
	GLuint m_nRenderFramebufferId;
	GLuint m_nResolveTextureId;
	GLuint m_nResolveFramebufferId;
};

class VR
{
public:
	// constructor
	VR();

	void InitVR();
	void Shutdown();

	void CreateFrameBuffers(int, int, FramebufferDesc&);
	void GetRecommendedRenderTargetSize(int&, int&);

	glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye);
	glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye);
	glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye);
	void SetupCameras();
	void UpdateHMDMatrixPose();

	void submitFramesOpenGL(GLint, GLint);

private:
	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t, vr::TrackedDeviceProperty, vr::TrackedPropertyError* = NULL);
	glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t&);
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	vr::IVRSystem* _hmd = NULL;
	uint32_t _width, _height;
	vr::IVRRenderModels* _renderModel;
	float _near, _far;

	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;
	glm::mat4 m_mat4eyePosLeft;
	glm::mat4 m_mat4eyePosRight;
	glm::mat4 m_mat4HMDPose;
};

#endif // !VR_H