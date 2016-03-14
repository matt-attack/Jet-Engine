

#ifdef _WIN32
#include <d3dx9.h>
#else
#include "../jni/glm/gtc/matrix_transform.hpp"
#include "../jni/glm/gtx/matrix_operation.hpp"
#include "../jni/glm/glm.hpp"
#include "../jni/glm/core/func_matrix.hpp"
#include "../jni/glm/gtc/matrix_transform.hpp"
#include "../jni/glm/gtc/type_ptr.hpp"
#endif

#include "camera.h"
#ifndef MATT_SERVER
#include "Graphics/CRenderer.h"
#endif
#include "Math/Quaternion.h"

CCamera::CCamera()
{
	_minXang = -1.5707963f;
	_maxXang = 1.5707963f;
	this->perspective = true;

	_near = 0.1f;
	_far = 2000.0f;
	this->quat = Quaternion::IDENTITY;//.FromAngleAxis(0.0f, Vec3(1,0,0));
	this->_lookAt = Vec3(0,1,0);
	this->_right = Vec3(0,0,1);
	this->_upDir = Vec3(1,0,0);
};

void CCamera::orthoProjection(float l, float r, float b, float t, float n, float f)
{
	this->perspective = false;
	this->_projectionMatrix = Matrix4::OrthographicOffCenterLHMatrix(l,r,b,t,n,f);
}

void CCamera::setPos(const Vec3 pos)//temporary fix
{
	this->_pos = pos;
};

void CCamera::setUp(const Vec3 pos)
{
	this->_upDir = pos;
};

void CCamera::setLookAt(const Vec3 pos)
{
	this->_lookAt = pos;
};

void CCamera::setFOV(const float fov)
{
	this->_fov = fov;
}

void CCamera::setFar(float Far)
{
	this->_far = Far;
}

void CCamera::setAspectRatio(const float ratio)
{
	this->_aspectRatio = ratio;
}

void CCamera::doLookAt(Vec3 at, Vec3 up)
{

	this->_matrix = _matrix.LookAtLHMatrix(this->_pos, at, up);

	Matrix4 matrix = this->_matrix;
	this->_right.x = -matrix._m44[0][0];
	this->_right.y = -matrix._m44[1][0];
	this->_right.z = -matrix._m44[2][0];
	this->_upDir.x = matrix._m44[0][1];
	this->_upDir.y = matrix._m44[1][1];
	this->_upDir.z = matrix._m44[2][1];
	this->_lookAt.x = -matrix._m44[0][2];
	this->_lookAt.y = -matrix._m44[1][2];
	this->_lookAt.z = -matrix._m44[2][2];

	this->BuildViewFrustum();
}

void CCamera::perspectiveProjection()
{
	this->perspective = true;
#if 1//ndef ANDROID
	Matrix4 mat;
	mat = mat.PerspectiveFovLHMatrix(this->_fov, this->_aspectRatio, 0.1f, this->_far);//Perspective(this->_fov,this->_aspectRatio,0.1f,this->_far);
#ifdef ANDROID
	mat = mat.PerspectiveFovLHMatrixOpenGL(this->_fov, this->_aspectRatio, 0.1f, this->_far);
#endif
	this->_projectionMatrix = mat;
	//if (renderer)
	//renderer->projection = mat;
#else
	Matrix4 mat6,t,s;//fixme
	mat6 = mat6.PerspectiveFovLHMatrix(this->_fov, this->_aspectRatio, 0.1f, this->_far);//*mat6.ScaleMatrix(Vec3(1.0f,1.0f,-1.0f));//Perspective(this->_fov,this->_aspectRatio,0.1f,this->_far);
	glm::mat4 pe = glm::perspective(this->_fov*180.0f/3.141592653589f, this->_aspectRatio, this->_near, this->_far);
	glm::mat4 p = glm::scale(pe, glm::vec3(-1,1,1));
	this->_projectionMatrix = *(Matrix4*)&p;
	//this->_projectionMatrix = this->_projectionMatrix.Transpose();
	if (didla)
	{
		//Matrix4 mat;
		//mat = mat.PerspectiveFovLHMatrix(this->_fov, this->_aspectRatio, 0.1f, this->_far);//Perspective(this->_fov,this->_aspectRatio,0.1f,this->_far);
		this->_projectionMatrix = mat6;
	}
	if (renderer)
		renderer->projection = this->_projectionMatrix;
#endif

}

void CCamera::doMatrix()
{
	//Vec3 cameraFinalTarget = Vec3(1,0,0);
	//Vec3 cameraRotatedUpVector = Vec3(0,1,0);

	Matrix4 t = Matrix4::Identity();
	this->quat.ToRotationMatrix(t);
	//cameraFinalTarget = t*cameraFinalTarget + this->_pos;
	//cameraRotatedUpVector = t*cameraRotatedUpVector;

	Matrix4 newv = Matrix4::TranslationMatrix(-this->_pos)*t.Transpose();//.Inverse();//Transpose();
	//newv = newv.LookAtLHMatrix(this->_pos, cameraFinalTarget, cameraRotatedUpVector);

	this->_matrix = newv;

	Matrix4 matrix = this->_matrix;
	this->_right.x = matrix._m44[0][0];
	this->_right.y = matrix._m44[1][0];
	this->_right.z = matrix._m44[2][0];
	this->_upDir.x = matrix._m44[0][1];
	this->_upDir.y = matrix._m44[1][1];
	this->_upDir.z = matrix._m44[2][1];
	this->_lookAt.x = matrix._m44[0][2];
	this->_lookAt.y = matrix._m44[1][2];
	this->_lookAt.z = matrix._m44[2][2];

	BuildViewFrustum();
};

void CCamera::doMatrixNoRot()
{
	Vec3 t = this->_pos + this->_lookAt;
	Matrix4 newv = newv.LookAtLHMatrix(this->_pos, t, this->_upDir);
	//D3DXMatrixLookAtLH(&_matrix, (D3DXVECTOR3*)&this->_pos/*&this->_pos*/, (D3DXVECTOR3*)&cameraFinalTarget, (D3DXVECTOR3*)&cameraRotatedUpVector);

	this->_matrix = newv;

	BuildViewFrustum();
};

void CCamera::applyCam()//kinda pointless to do all the time, but its pretty cheap :/
{
#ifndef MATT_SERVER
	if (renderer)
	{
		renderer->SetMatrix(VIEW_MATRIX, &this->_matrix);
		renderer->SetMatrix(PROJECTION_MATRIX, &this->_projectionMatrix);
	}
#endif
};

bool CCamera::SphereInFrustum( Vec3* pPosition, float radius )
{
	for (int i = 0; i < 6; i++)
	{
		if (m_frustum[i].Dot(*pPosition) < -radius)
		{
			// Outside the frustum, reject it!
			return false;
		}
	}
	return true;
};

//dist normal.dotProduct(point) + distance(of plane)
bool CCamera::BoxInFrustum(const AABB &b)
{
	Vec3 P;
	Vec3 Q;
	//for each plane do ...
	for(int i=0; i < 6; i++) {
		//      N  *Q                    *P
		//      | /                     /
		//      |/                     /
		// -----/----- Plane     -----/----- Plane
		//     /                     / |
		//    /                     /  |
		//   *P                    *Q  N
		//
		// PQ forms diagonal most closely aligned with plane normal.

		// For each frustum plane, find the box diagonal (there are four main
		// diagonals that intersect the box center point) that points in the
		// same direction as the normal along each axis (i.e., the diagonal
		// that is most aligned with the plane normal).  Then test if the box
		// is in front of the plane or not.

		for(int j = 0; j < 3; ++j)
		{
			// Make PQ point in the same direction as the plane normal on this axis.
			if( m_frustum[i][j] >= 0.0f )
			{
				P[j] = b.min[j];
				Q[j] = b.max[j];
			}
			else
			{
				P[j] = b.max[j];
				Q[j] = b.min[j];
			}
		}

		// If box is in negative half space, it is behind the plane, and thus, completely
		// outside the frustum.  Note that because PQ points roughly in the direction of the
		// plane normal, we can deduce that if Q is outside then P is also outside--thus we
		// only need to test Q.
		if( m_frustum[i].Dot(Q) < 0.0f)//D3DXPlaneDotCoord(&m_frustum[i], (D3DXVECTOR3*)&Q) < 0.0f ) // outside
			return false;
	}
	return true;
};

AABB CCamera::GetFrustumAABB()
{
	float yfar = this->_far*tan(this->_fov/2.0f);
	float xfar = yfar*this->_aspectRatio;
	float ztotal = this->_far - this->_near;
	float ynear = this->_near*tan(this->_fov/2.0f);
	float xnear = ynear*this->_aspectRatio;

	Vec3 boxCorners[8];
	boxCorners[0] = this->_pos+this->_right*xnear+this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
	boxCorners[1] = this->_pos+this->_right*xnear-this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
	boxCorners[2] = this->_pos+this->_right*xfar+this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[3] = this->_pos-this->_right*xnear-this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
	boxCorners[4] = this->_pos-this->_right*xnear+this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
	boxCorners[5] = this->_pos+this->_right*xfar-this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[6] = this->_pos-this->_right*xfar+this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[7] = this->_pos-this->_right*xfar-this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);

	AABB bb(0,0,0,0,0,0);
	for (int i = 0; i < 8; i++)
	{
		bb.FitPoint(boxCorners[i]);
	}
	return bb;
}

bool CCamera::BoxInFrustumSidesAndFar(const AABB &b)
{
	Vec3 P;
	Vec3 Q;
	//for each plane do ...
	for(int i = 1; i < 6; i++) {
		//      N  *Q                    *P
		//      | /                     /
		//      |/                     /
		// -----/----- Plane     -----/----- Plane
		//     /                     / |
		//    /                     /  |
		//   *P                    *Q  N
		//
		// PQ forms diagonal most closely aligned with plane normal.

		// For each frustum plane, find the box diagonal (there are four main
		// diagonals that intersect the box center point) that points in the
		// same direction as the normal along each axis (i.e., the diagonal
		// that is most aligned with the plane normal).  Then test if the box
		// is in front of the plane or not.

		for(int j = 0; j < 3; ++j)
		{
			// Make PQ point in the same direction as the plane normal on this axis.
			if(m_frustum[i][j] >= 0.0f)
			{
				P[j] = b.min[j];
				Q[j] = b.max[j];
			}
			else
			{
				P[j] = b.max[j];
				Q[j] = b.min[j];
			}
		}

		// If box is in negative half space, it is behind the plane, and thus, completely
		// outside the frustum.  Note that because PQ points roughly in the direction of the
		// plane normal, we can deduce that if Q is outside then P is also outside--thus we
		// only need to test Q.
		if (m_frustum[i].Dot(Q) < 0.0f)//D3DXPlaneDotCoord(&m_frustum[i], (D3DXVECTOR3*)&Q) < 0.0f ) // outside
			return false;
	}
	return true;
}

bool CCamera::BoxInFrustumSides(const AABB &b)
{
	Vec3 P;
	Vec3 Q;
	//for each plane do ...
	for(int i=2; i < 6; i++) {
		//      N  *Q                    *P
		//      | /                     /
		//      |/                     /
		// -----/----- Plane     -----/----- Plane
		//     /                     / |
		//    /                     /  |
		//   *P                    *Q  N
		//
		// PQ forms diagonal most closely aligned with plane normal.

		// For each frustum plane, find the box diagonal (there are four main
		// diagonals that intersect the box center point) that points in the
		// same direction as the normal along each axis (i.e., the diagonal
		// that is most aligned with the plane normal).  Then test if the box
		// is in front of the plane or not.

		for(int j = 0; j < 3; ++j)
		{
			// Make PQ point in the same direction as the plane normal on this axis.
			if( m_frustum[i][j] >= 0.0f )
			{
				P[j] = b.min[j];
				Q[j] = b.max[j];
			}
			else
			{
				P[j] = b.max[j];
				Q[j] = b.min[j];
			}
		}

		// If box is in negative half space, it is behind the plane, and thus, completely
		// outside the frustum.  Note that because PQ points roughly in the direction of the
		// plane normal, we can deduce that if Q is outside then P is also outside--thus we
		// only need to test Q.
		if( m_frustum[i].Dot(Q) < 0.0f)//D3DXPlaneDotCoord(&m_frustum[i], (D3DXVECTOR3*)&Q) < 0.0f ) // outside
			return false;
	}
	return true;
};

void CCamera::BuildViewFrustum()
{
#ifndef MATT_SERVER
	if (this->perspective)
	{
		D3DXMATRIX viewProjection;
		D3DXMatrixMultiply( &viewProjection, (D3DXMATRIX*)&_matrix, (D3DXMATRIX*)&_projectionMatrix );

		// Left plane
		m_frustum[2].a = viewProjection._14 + viewProjection._11;//normal x
		m_frustum[2].b = viewProjection._24 + viewProjection._21;//normal y
		m_frustum[2].c = viewProjection._34 + viewProjection._31;//normal z
		m_frustum[2].d = viewProjection._44 + viewProjection._41;//distance

		// Right plane
		m_frustum[3].a = viewProjection._14 - viewProjection._11;
		m_frustum[3].b = viewProjection._24 - viewProjection._21;
		m_frustum[3].c = viewProjection._34 - viewProjection._31;
		m_frustum[3].d = viewProjection._44 - viewProjection._41;

		// Top plane
		m_frustum[4].a = viewProjection._14 - viewProjection._12;
		m_frustum[4].b = viewProjection._24 - viewProjection._22;
		m_frustum[4].c = viewProjection._34 - viewProjection._32;
		m_frustum[4].d = viewProjection._44 - viewProjection._42;

		// Bottom plane
		m_frustum[5].a = viewProjection._14 + viewProjection._12;
		m_frustum[5].b = viewProjection._24 + viewProjection._22;
		m_frustum[5].c = viewProjection._34 + viewProjection._32;
		m_frustum[5].d = viewProjection._44 + viewProjection._42;

		// Near plane
		m_frustum[0].a = viewProjection._13;
		m_frustum[0].b = viewProjection._23;
		m_frustum[0].c = viewProjection._33;
		m_frustum[0].d = viewProjection._43;

		// Far plane
		m_frustum[1].a = viewProjection._14 - viewProjection._13;
		m_frustum[1].b = viewProjection._24 - viewProjection._23;
		m_frustum[1].c = viewProjection._34 - viewProjection._33;
		m_frustum[1].d = viewProjection._44 - viewProjection._43;

		// Normalize planes
		for ( int i = 0; i < 6; i++ )
		{
			m_frustum[i].Normalize();//D3DXPlaneNormalize( &m_frustum[i], &m_frustum[i] );
		}
	}
	else
	{
		const Matrix4& proj = this->_projectionMatrix;
		float width = 2.0/proj.m_afEntry[0];
		float hight = 2.0/proj.m_afEntry[5];

		//if (proj.m_afEntry[15] == 0)
		//throw 7;

		float znear = -proj.m_afEntry[11]/proj.m_afEntry[10];//14/10
		float zfar = 1.0f/proj.m_afEntry[10] + znear;

		float a = proj.m_afEntry[5];
		float b = proj.m_afEntry[7];
		float top = (1.0f-b)/a;//(1.0f/a - 1.0f)*b;
		float bottom = -(b+1.0)/a;//-2.0f*b-top;
		if (b <= 0.00001f && b >= -0.00001f)
		{
			top = 1.0f/a;
			bottom = -top;
		}

		a = proj.m_afEntry[0];
		b = proj.m_afEntry[3];
		float right = (1.0f-b)/a;//(1.0f/a - 1.0f)*b;
		float left = -(b+1.0f)/a;//-2.0f*b-right;
		if (b <= 0.00001f && b >= -0.00001f)
		{
			right = 1.0f/a;
			left = -right;
		}

		Matrix4 v = this->_matrix.Inverse();
		Vec3 pos, dir;

		//left plane
		pos = Vec3(left, bottom, znear);
		dir = Vec3(1,0,0);
		pos = v*pos; dir = v*dir;
		m_frustum[2] = Plane(pos, dir);

		//right plane
		pos = Vec3(right, bottom, znear);
		dir = Vec3(-1,0,0);
		pos = v*pos; dir = v*dir;
		m_frustum[3] = Plane(pos, dir);

		//top plane
		pos = Vec3(right, top, znear);
		dir = Vec3(0,-1,0);
		pos = v*pos; dir = v*dir;
		m_frustum[4] = Plane(pos, dir);

		//bottom plane
		pos = Vec3(right, bottom, znear);
		dir = Vec3(0,1,0);
		pos = v*pos; dir = v*dir;
		m_frustum[5] = Plane(pos, dir);

		//near plane
		pos = Vec3(right, bottom, znear);
		dir = Vec3(0,0,1);
		pos = v*pos; dir = v*dir;
		m_frustum[0] = Plane(pos, dir);

		//far plane
		pos = Vec3(right, bottom, zfar);
		dir = Vec3(0,0,-1);
		pos = v*pos; dir = v*dir;
		m_frustum[1] = Plane(pos, dir);
	}
#endif
}