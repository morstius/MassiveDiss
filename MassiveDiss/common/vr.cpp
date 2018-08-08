#include "vr.h"

VR::VR() : _vrEnabled(false), _hmd(NULL)
{
	if (vr::VR_IsHmdPresent() && vr::VR_IsRuntimeInstalled())
	{
		BInitVR();
		BInitCompositor();
		CreateFramebuffers(_leftEyeDesc);
		CreateFramebuffers(_rightEyeDesc);
		_vrEnabled = true;
	}
}

void VR::Shutdown()
{
	if (_hmd)
	{
		vr::VR_Shutdown();
		_hmd = NULL;
	}
}

void VR::BInitVR()
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

bool VR::BInitCompositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	return true;
}

void VR::CreateFramebuffers(FramebufferDesc &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, _width, _height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, _width, _height, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Can't create framebuffer...\n");
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void VR::RenderStereoTarget(GLuint programID, GLuint textureID, ObjInfo& objInfo, std::vector<MtlObj>& textureLib, std::vector<GLuint>& textures)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, _leftEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, _width, _height);
	RenderScene(programID, textureID, objInfo, textureLib, textures, vr::Eye_Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, _leftEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _leftEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);

	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, _rightEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, _width, _height);
	RenderScene(programID, textureID, objInfo, textureLib, textures, vr::Eye_Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, _rightEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _rightEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void VR::RenderFrame(GLuint programID, GLuint textureID, ObjInfo& objInfo, std::vector<MtlObj>& textureLib, std::vector<GLuint>& textures)
{
	if (_hmd)
	{
		RenderStereoTarget(programID, textureID, objInfo, textureLib, textures);

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)_leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)_rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}
}

void VR::RenderScene(GLuint programID, GLuint textureID, ObjInfo& objInfo, std::vector<MtlObj>& textureLib, std::vector<GLuint>& textures, vr::Hmd_Eye nEye)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	bool bIsInputAvailable = _hmd->IsInputAvailable();

	if (bIsInputAvailable)
	{
		// draw the controller axis lines
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// activating the default texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);

			// switch to the texture that we want to use
			if (objInfo.txIdx >= 0 && textureLib[objInfo.txIdx].hasTexture)
				glBindTexture(GL_TEXTURE_2D, textures[textureLib[objInfo.txIdx].textNbr - 1]);
			else
				glBindTexture(GL_TEXTURE_2D, textures[0]);

			// texture sampler using texture unit 0
			glUniform1i(textureID, 0);

			// layout(location = 0) attribute buffer [vertices]
			glBindBuffer(GL_ARRAY_BUFFER, objInfo.vertexbuffer);
			glVertexAttribPointer(
				0,
				3,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);

			// layout(location = 1) attribute buffer [colors]
			glBindBuffer(GL_ARRAY_BUFFER, objInfo.uvbuffer);
			glVertexAttribPointer(
				1,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);

			// layout(location = 2) attribute buffer [normals]
			glBindBuffer(GL_ARRAY_BUFFER, objInfo.normalbuffer);
			glVertexAttribPointer(
				2,
				3,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);

			// index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objInfo.elembuffer);

			// turn on blending
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// draw the triangles
			glDrawElements(
				GL_TRIANGLES,
				(GLsizei)objInfo.indices.size(),
				GL_UNSIGNED_INT,
				(void*)0
			);

			// disable blending
			glDisable(GL_BLEND);
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

	// ----- Render Model rendering -----
	/*glUseProgram(m_unRenderModelProgramID);

	for (EHand eHand = Left; eHand <= Right; ((int&)eHand)++)
	{
		if (!m_rHand[eHand].m_bShowController || !m_rHand[eHand].m_pRenderModel)
			continue;

		const Matrix4 & matDeviceToTracking = m_rHand[eHand].m_rmat4Pose;
		Matrix4 matMVP = GetCurrentViewProjectionMatrix(nEye) * matDeviceToTracking;
		glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

		m_rHand[eHand].m_pRenderModel->Draw();
	}*/

	glUseProgram(0);
}