#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include "Math/geom.h"

#include "Math/Quaternion.h"
#include "Math/AABB.h"

class Parent;

class CCamera
{
	bool perspective;
public:
	float _fov;
	float _aspectRatio;
	float _near;
	float _far;

	Plane m_frustum[6];

	Matrix4 _matrix;
	Matrix4 _projectionMatrix;

	Vec3 _upDir;
	Vec3 _pos;
	Vec3 _right;
	Vec3 _lookAt;

	Quaternion quat;

	float _minXang;
	float _maxXang;

	Parent* parent;

	void setFOV(float fov);
	void setFar(float far);
	void setAspectRatio(float ratio);
	void setPos(Vec3 pos);
	void setUp(Vec3 pos);
	void setLookAt(Vec3 pos);

	bool SphereInFrustum(Vec3* pPosition, float radius);
	bool BoxInFrustum(const AABB &b);
	bool BoxInFrustumSides(const AABB &b);
	bool BoxInFrustumSidesAndFar(const AABB &b);

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

	void doMatrix();
	void doMatrixNoRot();

	//setup project matrix
	void perspectiveProjection();
	void orthoProjection(float left, float right, float bottom, float top, float near, float far);


	void doLookAt(Vec3 at, Vec3 up);
	void BuildViewFrustum();

	void GetCorners(Vec3* corners)
	{
		corners[0] = IntersectionPoint(m_frustum[0], m_frustum[2], m_frustum[4]);
		corners[1] = IntersectionPoint(m_frustum[0], m_frustum[3], m_frustum[4]);
		corners[2] = IntersectionPoint(m_frustum[0], m_frustum[3], m_frustum[5]);
		corners[3] = IntersectionPoint(m_frustum[0], m_frustum[2], m_frustum[5]);
		corners[4] = IntersectionPoint(m_frustum[1], m_frustum[2], m_frustum[4]);
		corners[5] = IntersectionPoint(m_frustum[1], m_frustum[3], m_frustum[4]);
		corners[6] = IntersectionPoint(m_frustum[1], m_frustum[3], m_frustum[5]);
		corners[7] = IntersectionPoint(m_frustum[1], m_frustum[2], m_frustum[5]);
	}

	Vec3 IntersectionPoint(Plane& a, Plane& b, Plane& c)
	{
		Vec3 v1, v2, v3;
		Vec3 cross;
		cross = Vec3(-a.a,-a.b,-a.c).cross(-Vec3(c.a,c.b,c.c));//Vec3.cross(b.Normal, c.Normal, out cross);
		float f;
		f = Vec3(-a.a,-a.b,-a.c).dot(cross);//Vec3.dot(ref a.Normal, ref cross, out f);
		f *= -1.0f;
		cross = Vec3(-b.a,-b.b,-b.c).cross(-Vec3(c.a,c.b,c.c));//Vec3.cross(ref b.Normal, ref c.Normal, out cross);
		v1 = cross*a.d;//Vector3.Multiply(ref cross, a.D, out v1);
		//v1 = (a.D * (Vector3.Cross(b.Normal, c.Normal)));
		cross = Vec3(-c.a,-c.b,-c.c).cross(-Vec3(a.a,a.b,a.c));//cross = Vector3.Cross(ref c.Normal, ref a.Normal, out cross);
		v2 = cross*b.d;//Vector3.Multiply(ref cross, b.D, out v2);
		//v2 = (b.D * (Vector3.Cross(c.Normal, a.Normal)));
		cross = Vec3(-a.a,-a.b,-a.c).cross(-Vec3(b.a,b.b,b.c));//Vector3.Cross(ref a.Normal, ref b.Normal, out cross);
		v3 = cross*c.d;//Vector3.Multiply(ref cross, c.D, out v3);
		//v3 = (c.D * (Vector3.Cross(a.Normal, b.Normal)));
		Vec3 result;
		result.x = (v1.x + v2.x + v3.x) / f;
		result.y = (v1.y + v2.y + v3.y) / f;
		result.z = (v1.z + v2.z + v3.z) / f;
		return result;
	}

	CCamera();
};

#endif
