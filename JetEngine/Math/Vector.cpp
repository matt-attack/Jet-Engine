#include "Vector.h"
#include "Matrix.h"

Vec3::Vec3(const Vec4 &v) : x(v.x), y(v.y), z(v.z) {};

Vec3::Vec3(const Vec3d &v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

/*void Vec3::abs() 
{ 
	x = ::abs(x); y = ::abs(y); z = ::abs(z); 
}*/

Vec3* Vec3TransformCoord(Vec3 *pout, const  Vec3 *pv, const Matrix4f *pm)
{
	float norm;

	norm = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] *pv->z + pm->m[3][3];

	if ( norm )
	{
		pout->x = (pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0]) / norm;
		pout->y = (pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1]) / norm;
		pout->z = (pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2]) / norm;
	}
	else
	{
		pout->x = 0.0f;
		pout->y = 0.0f;
		pout->z = 0.0f;
	}
	return pout;
}

Vec3 Vec3::toscreen(/*D3DXVECTOR3 *pout, */const Vec3 *pv, const Viewport *pviewport, const Matrix4f *pprojection, const Matrix4f *pview, const Matrix4f *pworld)
{
	Matrix4f m1, m2;
	Vec3 vec;
	Vec3 out;
	Vec3* pout = &out;
	m1 = *pworld*(*pview);
	m2 = m1*(*pprojection);
	//D3DXMatrixMultiply(&m1, (D3DXMATRIX*)pworld, (D3DXMATRIX*)pview);
	//D3DXMatrixMultiply((D3DXMATRIX*)&m2, (D3DXMATRIX*)&m1, (D3DXMATRIX*)pprojection);
	//m2 = m2.Transpose();
	//D3DXMatrixTranspose(&m2, &m2);
	//vec = *pprojection**pview**pworld**pv;

	//vec = (Vec3)(m2*((Vec4)*pv));
	Vec3TransformCoord(&vec, pv, &m2);
	pout->x = pviewport->X + ( 1.0f + vec.x ) * pviewport->Width / 2.0f;
	pout->y = pviewport->Y + ( 1.0f - vec.y ) * pviewport->Height / 2.0f;
	pout->z = pviewport->MinZ + vec.z * ( pviewport->MaxZ - pviewport->MinZ );
	return out;
}