#include "vr.h"

VR::VR() : _hmd(NULL)
{
	// checking if hmd is present and that the VR runtime is installed
	if (vr::VR_IsHmdPresent() && vr::VR_IsRuntimeInstalled())
	{	
		// init vr
		InitVR();

		// checking compositor
		if (!vr::VRCompositor())
		{
			throw std::runtime_error("Unable to initialize VR compositor!\n ");
		}

		// getting the recommended render target width and height
		_hmd->GetRecommendedRenderTargetSize(&_width, &_height);
	}

	// setting the near and far settings to be used later
	_near = 0.01f;
	_far = 100.0f;
}

void VR::GetRecommendedRenderTargetSize(int& width, int& height)
{
	// returning the recommended width and height acquired in constructor
	width = _width;
	height = _height;
}

void VR::CreateFrameBuffers(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	// generate and bind framebuffer
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	// generate and bind framebuffer with multisample
	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	// generate multisample color texture
	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	// generate resolve buffer to blit over to
	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	// generate resolve texture with some filtering
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

	// bind default 0 buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	printf("Created framebuffer...\n");
}

void VR::Shutdown()
{
	// shutdown if hmd is there
	if (_hmd)
	{
		vr::VR_Shutdown();
		_hmd = NULL;
	}
}

void VR::InitVR()
{
	// loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	_hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	// checking for init errors
	if (eError != vr::VRInitError_None)
	{
		_hmd = NULL;
		char buf[1024];
		printf( "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
	}

	_renderModel = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);

	// getting generic interface
	if (eError != vr::VRInitError_None)
	{
		_hmd = NULL;
		char buf[1024];
		printf("Unable to get generic interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
	}

	std::string m_strDriver = "No Driver";
	std::string m_strDisplay = "No Display";

	// getting driver and display string to print out, for info
	m_strDriver = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strDisplay = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	printf("VR connect, device %s %s.\n", m_strDriver.c_str(), m_strDisplay.c_str());
}

std::string VR::GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	// get tracked device property
	uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);

	if (unRequiredBufferLen == 0)
	{
		return "";
	}

	// setting the string with the info and returning it
	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string res = pchBuffer;
	delete[] pchBuffer;
	return res;
}

void VR::SubmitFramesOpenGL(GLint leftEye, GLint rightEye)
{
	// if we don't have a hmd then throw exception
	if (!_hmd)
	{
		throw std::runtime_error("Error : presenting frames when VR system handle is NULL");
	}

	// waiting for poses
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

	// setting the left and right eye textures
	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftEye, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightEye, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

	// submit left and right eye
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	glFinish();

	// post present handoff
	vr::VRCompositor()->PostPresentHandoff();
}

glm::mat4 VR::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!_hmd)
	{
		return glm::mat4();
	}
	// get projection matrix from hmd
	vr::HmdMatrix44_t mat = _hmd->GetProjectionMatrix(nEye, _near, _far);

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

glm::mat4 VR::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if (!_hmd)
	{
		return glm::mat4();
	}
	// get eye to head transform from hmd
	vr::HmdMatrix34_t matEyeRight = _hmd->GetEyeToHeadTransform(nEye);
	glm::mat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0
	);

	return glm::inverse(matrixObj);
}

glm::mat4 VR::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	// converting a 3x4 to a 4x4 matrix
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}

void VR::UpdateHMDMatrixPose()
{
	if (!_hmd)
		return;

	// waiting for poses
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	int m_iValidPoseCount = 0;
	std::string m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice] == 0)
			{
				switch (_hmd->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	// checking if pose is valid
	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		glm::inverse(m_mat4HMDPose);
	}
}

glm::mat4 VR::GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
{
	glm::mat4 matMVP;

	// getting the view projectoin matrix according to eye
	if (nEye == vr::Eye_Left)
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if (nEye == vr::Eye_Right)
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
	}
	return matMVP;
}

void VR::SetupCameras()
{
	// setting up the projection and eye position matrices per eye
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
	m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

void VR::CleanupFrameBuffers(FramebufferDesc& framebufferDesc)
{
	// cleanup framebuffers
	glDeleteFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glDeleteFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);

	// cleanup render buffer
	glDeleteRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	
	// cleanup textures
	glDeleteTextures(1, &framebufferDesc.m_nRenderTextureId);
	glDeleteTextures(1, &framebufferDesc.m_nResolveTextureId);
}