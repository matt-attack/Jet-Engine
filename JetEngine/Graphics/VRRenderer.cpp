#include "VRRenderer.h"


#include "RenderTexture.h"

#pragma comment (lib, "../../openvr/lib/win32/openvr_api.lib")

bool VRRenderer::Init(Window* win, int xres, int yres, bool fake)
{
	if (fake)
	{
		render_height = render_width = 1000;
		goto finish;
	}
	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;

	hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
	{
		hmd = NULL;
		char buf[1024];
		//sprintf_s(buf, ARRAYSIZE(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		//std::string temp(buf);
		//std::wstring wtemp(temp.begin(), temp.end());
		//MessageBoxW(0, wtemp.c_str(), L"VR_Init Failed", 0);
		return false;
	}

	hmd->GetRecommendedRenderTargetSize(&render_width, &render_height);

	printf("width = %d, height = %d", render_width, render_height);

	this->render_models = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!render_models)
	{
		hmd = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, ARRAYSIZE(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		std::string temp(buf);
		std::wstring wtemp(temp.begin(), temp.end());
		MessageBoxW(0, wtemp.c_str(), L"VR_Init Failed", NULL);
		return false;
	}

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

finish:
	// Create Context and shizzle
	CRenderer::Init(win, xres, yres);

	left_eye = CRenderTexture::Create(render_width, render_height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);
	right_eye = CRenderTexture::Create(render_width, render_height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		tracked_device_types[i] = 0;

	return true;
}

void VRRenderer::Clear(float a, float r, float g, float b)
{
	left_eye->Clear(a, r, g, b);
	right_eye->Clear(a, r, g, b);
}

void VRRenderer::BindEye(Eye eye)
{
	auto rt = eye == Eye::Left_Eye ? this->left_eye : this->right_eye;

	renderer->SetRenderTarget(0, rt);

	Viewport vp;
	vp.Height = this->render_height;
	vp.Width = this->render_width;
	vp.X = vp.Y = 0;
	vp.MaxZ = 1.0;
	vp.MinZ = 0.0;
	renderer->SetViewport(&vp);
}

void VRRenderer::SubmitTextures()
{
	if (fake)
	{
		return;
	}
	vr::Texture_t leftEyeTexture = { left_eye->texture, vr::TextureType_DirectX, vr::ColorSpace_Auto };
	vr::EVRCompositorError error1 = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { right_eye->texture, vr::TextureType_DirectX, vr::ColorSpace_Auto };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	if (error1)
	{
		char str[500];
		sprintf(str, "error is %d \n", error1);
		OutputDebugStringA(str);
	}
}

void VRRenderer::VisualizeWindow()
{
	//render the textures to our window's back buffer
	Rect r;
	r.left = r.top = 0;
	r.right = this->xres / 2;
	r.bottom = this->yres / 3;
	this->SetPixelTexture(0, left_eye);
	this->DrawRect(&r, COLOR_ARGB(255, 255, 255, 255));

	r.left = this->xres / 2;
	r.right = this->xres;
	this->SetPixelTexture(0, right_eye);
	this->DrawRect(&r, COLOR_ARGB(255, 255, 255, 255));

	this->SetPixelTexture(0, 0);
}

void VRRenderer::Present()
{
	this->VisualizeWindow();

	if (this->chain)
	{
		this->chain->Present(this->vsync ? 1 : 0, 0);
	}

	this->SubmitTextures();

	// Now we wait for the next HMD poses
	this->UpdatePoses();
}

Matrix4 GetHMDMatrixPoseEye(vr::IVRSystem* hmd, vr::Hmd_Eye nEye)
{
	if (!hmd)
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight = hmd->GetEyeToHeadTransform(nEye);
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);

	return matrixObj.Inverse();
}

Matrix4 GetHMDMatrixProjectionEye(vr::IVRSystem* hmd, vr::Hmd_Eye nEye)
{
	if (!hmd)
		return Matrix4();

	vr::HmdMatrix44_t mat = hmd->GetProjectionMatrix(nEye, 0.1, 5000);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

void VRRenderer::GetLeftEyePMatrix(Matrix4& mat)
{
	mat = GetHMDMatrixProjectionEye(hmd, vr::Eye_Left);
//todo use these and matMVP = m_mat4ProjectionRight * (m_mat4eyePosRight *  m_mat4HMDPose); to get pose right pair is the view matrix
}

void VRRenderer::GetLeftEyeVMatrix(Matrix4& mat)
{
	mat = GetHMDMatrixPoseEye(hmd, vr::Eye_Left);
}


void VRRenderer::GetRightEyePMatrix(Matrix4& mat)
{
	mat = GetHMDMatrixProjectionEye(hmd, vr::Eye_Right);
}

void VRRenderer::GetRightEyeVMatrix(Matrix4& mat)
{
	mat = GetHMDMatrixPoseEye(hmd, vr::Eye_Right);
}

Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}

void VRRenderer::UpdatePoses()
{
	if (fake)
	{
		// return our mouselook/kb camera
		this->valid_poses = 1;
		//this->hmd_view = whatever it is
		return;
	}

	vr::VRCompositor()->WaitGetPoses(tracked_poses_vr, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	valid_poses = 0;
	std::string m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (tracked_poses_vr[nDevice].bPoseIsValid)
		{
			valid_poses++;
			tracked_poses[nDevice] = ConvertSteamVRMatrixToMatrix4(tracked_poses_vr[nDevice].mDeviceToAbsoluteTracking);
			if (tracked_device_types[nDevice] == 0)
			{
				switch (hmd->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        tracked_device_types[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               tracked_device_types[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           tracked_device_types[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    tracked_device_types[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: tracked_device_types[nDevice] = 'T'; break;
				default:                                       tracked_device_types[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += tracked_device_types[nDevice];
		}
	}

	if (tracked_poses_vr[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		hmd_view = tracked_poses[vr::k_unTrackedDeviceIndex_Hmd].Inverse();//this is equal to the inverse of the view matrix
		//as it is camera to scene, we want scene to camera (or do I have this opposite? who knows)
	}
	else
	{
		printf("pose not valid");
	}
}

Matrix4 VRRenderer::GetControllerPose(int id)
{
	return this->tracked_poses[id];
}

int VRRenderer::GetNumTrackedPoses()
{
	return this->valid_poses;
}