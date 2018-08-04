#pragma once

#include "CRenderer.h"

#include "../../../openvr/headers/openvr.h"

/*namespace vr
{
	class IVRRenderModels;
	class IVRSystem;
}*/

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

	void UpdatePoses();

public:

	CRenderTexture *left_eye, *right_eye;

	bool Init(Window* win, int xres, int yres);

	void SubmitTextures();

	void VisualizeWindow();

	Matrix4 GetHMDPose()
	{
		return this->hmd_view;
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