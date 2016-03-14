#include "camera.h"

#include "Graphics/CRenderer.h"

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

extern CRenderer* renderer;

CCamera::CCamera()
{
	_minXang = -1.5707963f;
	_maxXang = 1.5707963f;

	_near = 0.1f;
	_far = 2000.0f;
	this->quat = Quaternion::IDENTITY;//.FromAngleAxis(0.0f, Vec3(1,0,0));
	this->_lookAt = Vec3(0,0,0);
	this->_right = Vec3(0,0,0);
	this->_upDir = Vec3(0,0,0);
};

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
#ifdef ANDROID
	didla = true;
#endif
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

void CCamera::doProjection()
{
#ifndef ANDROID
	Matrix4 mat;
	mat = mat.PerspectiveFovLHMatrix(this->_fov, this->_aspectRatio, 0.1f, this->_far);//Perspective(this->_fov,this->_aspectRatio,0.1f,this->_far);
	this->_projectionMatrix = mat;
#else
	Matrix4 mat6,t,s;//fixme
	mat6 = mat6.PerspectiveFovLHMatrix(this->_fov, this->_aspectRatio, 0.1f, this->_far);//*mat6.ScaleMatrix(Vec3(1.0f,1.0f,-1.0f));//Perspective(this->_fov,this->_aspectRatio,0.1f,this->_far);
	glm::mat4 pe = glm::perspective(this->_fov*180.0f/3.141592653589f, this->_aspectRatio, this->_near, this->_far);
	glm::mat4 p = glm::scale(pe, glm::vec3(-1,1,1));
	this->_projectionMatrix = *(Matrix4*)&p;
	//this->_projectionMatrix = this->_projectionMatrix.Transpose();
	//if (didla)
	{
		//Matrix4 mat;
		//mat = mat.PerspectiveFovLHMatrix(this->_fov, this->_aspectRatio, 0.1f, this->_far);//Perspective(this->_fov,this->_aspectRatio,0.1f,this->_far);
		this->_projectionMatrix = mat6;
	}
	if (renderer)
		renderer->projection = this->_projectionMatrix;
#endif
}

#include "Math\Quaternion.h"
void CCamera::doMatrix()
{
	Vec3 cameraFinalTarget = Vec3(1,0,0);
	Vec3 cameraRotatedUpVector = Vec3(0,1,0);

	Matrix4 t = t.Identity();
	this->quat.ToRotationMatrix(t);
	cameraFinalTarget = t*cameraFinalTarget + this->_pos;
	cameraRotatedUpVector = t*cameraRotatedUpVector;

	this->_matrix = Matrix4::LookAtLHMatrix(this->_pos, cameraFinalTarget, cameraRotatedUpVector);//*newv.ScaleMatrixXYZ(1,1,-1);

	Matrix4 matrix = this->_matrix;//.Transpose();//.Inverse();
	this->_right.x = -matrix._m44[0][0];
	this->_right.y = -matrix._m44[1][0];
	this->_right.z = -matrix._m44[2][0];
	this->_upDir.x = matrix._m44[0][1];
	this->_upDir.y = matrix._m44[1][1];
	this->_upDir.z = matrix._m44[2][1];
	this->_lookAt.x = -matrix._m44[0][2];
	this->_lookAt.y = -matrix._m44[1][2];
	this->_lookAt.z = -matrix._m44[2][2];

	BuildViewFrustum();
};

void CCamera::doMatrixNoRot()
{
	Matrix4 newv;
	Vec3 t = this->_pos + this->_lookAt;
	this->_matrix.SetTranslation(-this->_pos);
	//newv.SetTranslation(-this->_pos);
	//newv = newv.LookAtLHMatrix(this->_pos, t, this->_upDir);
	//D3DXMatrixLookAtLH(&_matrix, (D3DXVECTOR3*)&this->_pos/*&this->_pos*/, (D3DXVECTOR3*)&cameraFinalTarget, (D3DXVECTOR3*)&cameraRotatedUpVector);

	//this->_matrix = newv;

	BuildViewFrustum();
};

void CCamera::applyCam()//kinda pointless to do all the time, but its pretty cheap :/
{
#ifndef ANDROID
	renderer->d3ddev->SetTransform(D3DTS_PROJECTION, (D3DXMATRIX*)&this->_projectionMatrix);
	renderer->d3ddev->SetTransform(D3DTS_VIEW, (D3DXMATRIX*)&this->_matrix);
#endif

	if (renderer)
	{
		renderer->view = this->_matrix;
		renderer->projection = this->_projectionMatrix;
	}
};

bool CCamera::SphereInFrustum( Vec3* pPosition, float radius )
{
	for ( int i = 0; i < 6; i++ )
	{
		if ( m_frustum[i].Dot(*pPosition) < -radius)//D3DXPlaneDotCoord( &m_frustum[i], pPosition ) < -radius )
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

void CCamera::BuildViewFrustum()
{
	glm::mat4 viewProjection;
	// viewProjection = _matrix*_projectionMatrix;//glm::mul(&_matrix, &_projectionMatrix);//( &viewProjection, &_matrix, &_projectionMatrix );

	//viewProjection = _matrix*_projectionMatrix;
	// Left plane
	/*m_frustum[0].a = viewProjection[0][3] + viewProjection[0][0];//normal x
	m_frustum[0].b = viewProjection[1][3] + viewProjection[1][0];//normal y
	m_frustum[0].c = viewProjection[2][3] + viewProjection[2][0];//normal z
	m_frustum[0].d = viewProjection[3][3] + viewProjection[3][0];//distance

	// Right plane
	m_frustum[1].a = viewProjection[0][3] - viewProjection[0][0];
	m_frustum[1].b = viewProjection[1][3] - viewProjection[1][0];
	m_frustum[1].c = viewProjection[2][3] - viewProjection[2][0];
	m_frustum[1].d = viewProjection[3][3] - viewProjection[3][0];

	// Top plane
	m_frustum[2].a = viewProjection[0][3] - viewProjection[0][1];
	m_frustum[2].b = viewProjection[1][3] - viewProjection[1][1];
	m_frustum[2].c = viewProjection[2][3] - viewProjection[2][1];
	m_frustum[2].d = viewProjection[3][3] - viewProjection[3][1];

	// Bottom plane
	m_frustum[3].a = viewProjection[0][3] + viewProjection[0][1];
	m_frustum[3].b = viewProjection[1][3] + viewProjection[1][1];
	m_frustum[3].c = viewProjection[2][3] + viewProjection[2][1];
	m_frustum[3].d = viewProjection[3][3] + viewProjection[3][1];

	// Near plane
	m_frustum[4].a = viewProjection[0][2];
	m_frustum[4].b = viewProjection[1][2];
	m_frustum[4].c = viewProjection[2][2];
	m_frustum[4].d = viewProjection[3][2];

	// Far plane
	m_frustum[5].a = viewProjection[0][3] - viewProjection[0][2];
	m_frustum[5].b = viewProjection[1][3] - viewProjection[1][2];
	m_frustum[5].c = viewProjection[2][3] - viewProjection[2][2];
	m_frustum[5].d = viewProjection[3][3] - viewProjection[3][2];

	// Normalize planes
	for ( int i = 0; i < 6; i++ )
	{
	D3DXPlaneNormalize( &m_frustum[i], &m_frustum[i] );//normalize a b and c
	}*/
	float proj[16];
	float modl[16];
	float clip[16];
	float t;

	/* Get the current PROJECTION matrix from OpenGL */
	memcpy(&proj, (float*)&this->_projectionMatrix, 4*16);//glm::value_ptr(this->_projectionMatrix), 4*16);
	// glGetFloatv( GL_PROJECTION_MATRIX, proj );

	/* Get the current MODELVIEW matrix from OpenGL */
	memcpy(&modl, (float*)&this->_matrix, 4*16);//glm::value_ptr(this->_matrix), 4*16);
	//glGetFloatv( GL_MODELVIEW_MATRIX, modl );

	/* Combine the two matrices (multiply projection by modelview)    */
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	/* Extract the numbers for the RIGHT plane */
	m_frustum[0].a = clip[ 3] - clip[ 0];
	m_frustum[0].b = clip[ 7] - clip[ 4];
	m_frustum[0].c = clip[11] - clip[ 8];
	m_frustum[0].d = clip[15] - clip[12];

	/* Normalize the result */
	t = sqrt( m_frustum[0].a * m_frustum[0].a + m_frustum[0].b * m_frustum[0].b + m_frustum[0].c*m_frustum[0].c );
	m_frustum[0].a /= t;
	m_frustum[0].b /= t;
	m_frustum[0].c /= t;
	m_frustum[0].d /= t;

	/* Extract the numbers for the LEFT plane */
	m_frustum[1].a = clip[ 3] + clip[ 0];
	m_frustum[1].b = clip[ 7] + clip[ 4];
	m_frustum[1].c = clip[11] + clip[ 8];
	m_frustum[1].d = clip[15] + clip[12];

	/* Normalize the result */
	t = sqrt( m_frustum[1].a * m_frustum[1].a + m_frustum[1].b * m_frustum[1].b + m_frustum[1].c * m_frustum[1].c );
	m_frustum[1].a /= t;
	m_frustum[1].b /= t;
	m_frustum[1].c /= t;
	m_frustum[1].d /= t;

	/* Extract the BOTTOM plane */
	m_frustum[2].a = clip[ 3] + clip[ 1];
	m_frustum[2].b = clip[ 7] + clip[ 5];
	m_frustum[2].c = clip[11] + clip[ 9];
	m_frustum[2].d = clip[15] + clip[13];

	/* Normalize the result */
	t = sqrt( m_frustum[2].a * m_frustum[2].a + m_frustum[2].b * m_frustum[2].b + m_frustum[2].b * m_frustum[2].c );
	m_frustum[2].a /= t;
	m_frustum[2].b /= t;
	m_frustum[2].c /= t;
	m_frustum[2].d /= t;

	/* Extract the TOP plane */
	m_frustum[3].a = clip[ 3] - clip[ 1];
	m_frustum[3].b = clip[ 7] - clip[ 5];
	m_frustum[3].c = clip[11] - clip[ 9];
	m_frustum[3].d = clip[15] - clip[13];

	/* Normalize the result */
	t = sqrt( m_frustum[3].a * m_frustum[3].a + m_frustum[3].b * m_frustum[3].b + m_frustum[3].c* m_frustum[3].c );
	m_frustum[3].a /= t;
	m_frustum[3].b /= t;
	m_frustum[3].c /= t;
	m_frustum[3].d /= t;

	/* Extract the FAR plane */
	m_frustum[4].a = clip[ 3] - clip[ 2];
	m_frustum[4].b = clip[ 7] - clip[ 6];
	m_frustum[4].c = clip[11] - clip[10];
	m_frustum[4].d = clip[15] - clip[14];

	/* Normalize the result */
	t = sqrt( m_frustum[4].b * m_frustum[4].a + m_frustum[4].b * m_frustum[4].b + m_frustum[4].c* m_frustum[4].c );
	m_frustum[4].a /= t;
	m_frustum[4].b /= t;
	m_frustum[4].c /= t;
	m_frustum[4].d /= t;

	/* Extract the NEAR plane */
	m_frustum[5].a = clip[ 3] + clip[ 2];
	m_frustum[5].b = clip[ 7] + clip[ 6];
	m_frustum[5].c = clip[11] + clip[10];
	m_frustum[5].d = clip[15] + clip[14];

	/* Normalize the result */
	t = sqrt( m_frustum[5].a * m_frustum[5].a + m_frustum[5].b * m_frustum[5].b + m_frustum[5].c * m_frustum[5].c );
	m_frustum[5].a /= t;
	m_frustum[5].b /= t;
	m_frustum[5].c /= t;
	m_frustum[5].d /= t;
}