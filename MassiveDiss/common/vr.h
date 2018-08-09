#ifndef VR_H
#define VR_H

#include "GL/glew.h"
#include <openvr/headers/openvr.h>

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

	void submitFramesOpenGL(GLint, GLint, bool = false);

private:
	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t, vr::TrackedDeviceProperty, vr::TrackedPropertyError* = NULL);

	vr::IVRSystem* _hmd = NULL;
	uint32_t _width, _height;
	vr::IVRRenderModels* _renderModel;
};

#endif // !VR_H