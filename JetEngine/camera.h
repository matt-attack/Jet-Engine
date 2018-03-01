#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include "Math/geom.h"

#include "Math/Quaternion.h"
#include "Math/AABB.h"

class Parent;

class CCamera
{
	bool perspective;

	//frustum planes
	Plane m_frustum[6];

public:
	float _fov;
	float _aspectRatio;
	float _near;
	float _far;

	//current view and projection matrices
	Matrix4 _matrix;
	Matrix4 _projectionMatrix;

	//camera position
	Vec3 _pos;

	//current look directions, read only
private:
	Vec3 _upDir;
	Vec3 _right;
	Vec3 _lookAt;

public:
	Vec3 GetUp()
	{
		return this->_upDir;
	}

	Vec3 GetRight()
	{
		return this->_right;
	}

	Vec3 GetForward()
	{
		return this->_lookAt;
	}

	//camera rotation
	Quaternion quat;

	Parent* parent = 0;

	//FOV in radians
	void SetFOV(float fov_radians);
	void SetFar(float far);
	void SetNear(float near);
	void SetAspectRatio(float ratio);
	void SetPos(Vec3 pos);

	//frustum tests
	bool SphereInFrustum(Vec3* pPosition, float radius) const;
	bool BoxInFrustum(const AABB &b) const;
	bool BoxInFrustumSides(const AABB &b) const;
	bool BoxInFrustumSidesAndFar(const AABB &b) const;

	//builds the frustum
	void BuildViewFrustum();

	AABB GetFrustumAABB();

	Vec3 GetDirection(float xclip, float yclip)
	{
		float fov = _fov;// * 3.141592653589f/180.0f;

		float ytotal = _near*tan(fov/2.0f);
		float xtotal = ytotal*_aspectRatio;

		//get positions relative to cam's forward vector
		float x = xclip * xtotal;
		float y = yclip * ytotal;

		Vec3 xoff = _right*x;
		Vec3 yoff = _upDir*y;
		Vec3 zoff = _lookAt*_near;

		Vec3 worldpos = _pos + xoff + yoff + zoff;
		return (_pos - worldpos).getnormal();
	}

	//this builds the matrix using the current settings, overwriting a doLookAt
	void DoMatrix();
	void DoMatrixNoRot();

	//setup project matrix
	void PerspectiveProjection();
	void OrthoProjection(float left, float right, float bottom, float top, float near, float far);

	//this builds a lookat matrix and sets it as the matrix
	void DoLookAt(Vec3 at, Vec3 up);

	CCamera();
};

#endif
