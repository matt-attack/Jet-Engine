#pragma once

#include "CRenderer.h"

#include "../../../openvr/headers/openvr.h"

/*namespace vr
{
	class IVRRenderModels;
	class IVRSystem;
}*/

enum Eye
{
	Left_Eye,
	Right_Eye
};

class VRRenderer : public CRenderer
{
	unsigned int render_width, render_height;
	vr::IVRRenderModels* render_models;
	vr::IVRSystem* hmd;
	vr::TrackedDevicePose_t tracked_poses_vr[vr::k_unMaxTrackedDeviceCount];
	char tracked_device_types[vr::k_unMaxTrackedDeviceCount];
	int valid_poses;
	Matrix4 tracked_poses[vr::k_unMaxTrackedDeviceCount];
	Matrix4 hmd_view;

	//todo cache things in here
	Matrix4 left_project, right_projection;

	bool fake = false;

	void UpdatePoses();

public:

	// zero is HMD, one is left, two is right generally
	int GetNumTrackedPoses();

	Matrix4 GetControllerPose(int id);

	CRenderTexture *left_eye, *right_eye;

	void BindEye(Eye eye);

	bool Init(Window* win, int xres, int yres, bool fake = false);

	void Clear(float a, float r, float g, float b);

	void SubmitTextures();

	void VisualizeWindow();

	Matrix4 GetHMDPose()
	{
		Matrix4 hmd = this->hmd_view;
		hmd._m44[2][0] *= -1.0;
		hmd._m44[2][1] *= -1.0;
		hmd._m44[2][2] *= -1.0;
		hmd._m44[2][3] *= -1.0;
		return hmd;
	}

	void GetLeftEyePMatrix(Matrix4& mat);
	void GetLeftEyeVMatrix(Matrix4& mat);

	void GetRightEyePMatrix(Matrix4& mat);
	void GetRightEyeVMatrix(Matrix4& mat);

	virtual void Present();

	unsigned int Height()
	{
		return render_height;
	}

	unsigned int Width()
	{
		return render_width;
	}
};