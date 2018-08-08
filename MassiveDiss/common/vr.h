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

	void BInitVR();
	bool BInitCompositor();
	void Shutdown();

	void RenderStereoTarget(GLuint, GLuint, ObjInfo&, std::vector<MtlObj>&, std::vector<GLuint>&);
	void RenderFrame(GLuint, GLuint, ObjInfo&, std::vector<MtlObj>&, std::vector<GLuint>&);
	void RenderScene(GLuint, GLuint, ObjInfo&, std::vector<MtlObj>&, std::vector<GLuint>&, vr::Hmd_Eye);

	void CreateFramebuffers(FramebufferDesc&);

private:
	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t, vr::TrackedDeviceProperty, vr::TrackedPropertyError* = NULL);

	bool _vrEnabled = false;
	vr::IVRSystem* _hmd = NULL;
	uint32_t _width, _height;
	vr::IVRRenderModels* _renderModel;
	FramebufferDesc _leftEyeDesc;
	FramebufferDesc _rightEyeDesc;
};

#endif // !VR_H