#include "vr.h"

VR::VR() : _hmd(NULL)
{
	if (vr::VR_IsHmdPresent() && vr::VR_IsRuntimeInstalled())
	{	
		InitVR();

		if (!vr::VRCompositor())
		{
			throw std::runtime_error("Unable to initialize VR compositor!\n ");
		}

		_hmd->GetRecommendedRenderTargetSize(&_width, &_height);
	}
}

void VR::GetRecommendedRenderTargetSize(int& width, int& height)
{
	width = _width;
	height = _height;
}

void VR::CreateFrameBuffers(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Failed to create framebuffer...\n");
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	printf("Created framebuffer...\n");
}

void VR::Shutdown()
{
	if (_hmd)
	{
		vr::VR_Shutdown();
		_hmd = NULL;
	}
}

void VR::InitVR()
{
	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	_hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
	_hmd->GetRecommendedRenderTargetSize(&_width, &_height);

	if (eError != vr::VRInitError_None)
	{
		_hmd = NULL;
		char buf[1024];
		printf( "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
	}

	_renderModel = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);

	if (eError != vr::VRInitError_None)
	{
		_hmd = NULL;
		char buf[1024];
		printf("Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
	}

	std::string m_strDriver = "No Driver";
	std::string m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strDisplay = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	printf("VR connect, device %s %s.\n", m_strDriver.c_str(), m_strDisplay.c_str());
}

std::string VR::GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);

	if (unRequiredBufferLen == 0)
	{
		return "";
	}

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string res = pchBuffer;
	delete[] pchBuffer;
	return res;
}

void VR::submitFramesOpenGL(GLint leftEye, GLint rightEye, bool linear)
{
	if (!_hmd)
	{
		throw std::runtime_error("Error : presenting frames when VR system handle is NULL");
	}

	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftEye, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightEye, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	vr::VRCompositor()->PostPresentHandoff();
}