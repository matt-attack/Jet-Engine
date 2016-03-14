#include "AABB.h"

const bool AABBSweep
	(
	const VECTOR& Ea, //extents of AABB A
	const VECTOR& A0, //its previous position
	const VECTOR& A1, //its current position
	const VECTOR& Eb, //extents of AABB B
	const VECTOR& B0, //its previous position
	const VECTOR& B1, //its current position
	SCALAR& u0, //normalized time of first collision
	SCALAR& u1 //normalized time of second collision 
	)
{
	AABB A( A0 - Ea, A0 + Ea );//previous state of AABB A
	const AABB B( B0 - Eb, B0 + Eb);//previous state of AABB B
	const VECTOR va = A1 - A0;//displacement of A
	const VECTOR vb = B1 - B0;//displacement of B 

	//the problem is solved in A's frame of reference

	VECTOR v = vb - va;
	//relative velocity (in normalized time)

	VECTOR u_0(0,0,0);
	//first times of overlap along each axis

	VECTOR u_1(1,1,1);
	//last times of overlap along each axis

	//check if they were overlapping
	// on the previous frame
	if(A.Intersects(B) )
	{
		u0 = u1 = 0;
		return true;
	}

	//find the possible first and last times
	//of overlap along each axis
	for( int i=0 ; i<3 ; i++ )
	{
		if( A.max[i]<B.min[i] && v[i]<0 )
		{
			u_0[i] = (A.max[i] - B.min[i]) / v[i]; 
		}
		else if( B.max[i]<A.min[i] && v[i]>0 )
		{
			u_0[i] = (A.min[i] - B.max[i]) / v[i]; 
		}

		if( B.max[i]>A.min[i] && v[i]<0 )
		{
			u_1[i] = (A.min[i] - B.max[i]) / v[i]; 
		}
		else if( A.max[i]>B.min[i] && v[i]>0 )
		{
			u_1[i] = (A.max[i] - B.min[i]) / v[i]; 
		}
	}

	//possible first time of overlap
	u0 = mmax( u_0.x, mmax(u_0.y, u_0.z) );

	//possible last time of overlap
	u1 = mmin( u_1.x, mmin(u_1.y, u_1.z) );

	//they could have only collided if
	//the first time of overlap occurred
	//before the last time of overlap
	return u0 <= u1;
}

const bool AABBIntersectingSweptBox
	(
	const VECTOR& Ea, //extents of AABB A
	const VECTOR& A0, //its previous position
	const VECTOR& A1, //its current position
	const VECTOR& Eb, //extents of AABB B
	const VECTOR& B0, //its previous position
	const VECTOR& B1, //its current position
	SCALAR& u0, //normalized time of first collision
	SCALAR& u1 //normalized time of second collision 
	)
{
	AABB A( A0 - Ea, A0 + Ea );//previous state of AABB A
	const AABB B( B0 - Eb, B0 + Eb);//previous state of AABB B
	const VECTOR va = A1 - A0;//displacement of A
	const VECTOR vb = B1 - B0;//displacement of B 

	//the problem is solved in A's frame of reference

	VECTOR v = vb - va;
	//relative velocity (in normalized time)

	VECTOR u_0(0,0,0);
	//first times of overlap along each axis

	VECTOR u_1(1,1,1);
	//last times of overlap along each axis

	//check if they were overlapping
	// on the previous frame
	if(A.Intersects(B) )
	{
		u0 = u1 = 0;
		return true;
	}

	//find the possible first and last times
	//of overlap along each axis
	for( int i=0 ; i<3 ; i++ )
	{
		if( A.max[i]<B.min[i] && v[i]<0 )
		{
			u_0[i] = (A.max[i] - B.min[i]) / v[i]; 
		}
		else if( B.max[i]<A.min[i] && v[i]>0 )
		{
			u_0[i] = (A.min[i] - B.max[i]) / v[i]; 
		}

		if( B.max[i]>A.min[i] && v[i]<0 )
		{
			u_1[i] = (A.min[i] - B.max[i]) / v[i]; 
		}
		else if( A.max[i]>B.min[i] && v[i]>0 )
		{
			u_1[i] = (A.max[i] - B.min[i]) / v[i]; 
		}
	}

	//possible first time of overlap
	u0 = mmax( u_0.x, mmax(u_0.y, u_0.z) );

	//possible last time of overlap
	u1 = mmin( u_1.x, mmin(u_1.y, u_1.z) );

	//they could have only collided if
	//the first time of overlap occurred
	//before the last time of overlap
	return u0 <= u1;
}

/*CBox3D::CBox3D(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
this->max.x = xmax;
this->max.y = ymax;
this->max.z = zmax;
this->min.x = xmin;
this->min.y = ymin;
this->min.z = zmin;
};

CBox3D::CBox3D()
{

};

void CBox3D::CreateFromPoints(Vec3* a, int count)
{
this->min = a[0];
for (int i = 0; i < count; i++)
{
Vec3 p = a[i];
if (p.x > this->max.x)
this->max.x = p.x;
if (p.y > this->max.y)
this->max.y = p.y;
if (p.z > this->max.z)
this->max.z = p.z;

if (p.x < this->min.x)
this->min.x = p.x;
if (p.y < this->min.y)
this->min.y = p.y;
if (p.z < this->min.z)
this->min.z = p.z;
}
}

Vec3 CBox3D::GetVertexN(Vec3 normal)
{
//D3DXVECTOR3 n = D3DXVECTOR3(xmax,ymax,zmax);
//if (normal.x >= 0)
//	n.x = xmin;
//if (normal.y >=0)
//	n.y = ymin;
//if (normal.z >= 0)
//	n.z = zmin;
//return n;
return Vec3();
};

Vec3 CBox3D::GetVertexP(Vec3 normal)
{
//Vec3 p = Vec3(xmin,ymin,zmin);
//if (normal.x >= 0)
//	p.x = xmax;
//if (normal.y >=0)
//	p.y = ymax;
//if (normal.z >= 0)
//	p.z = zmax;
//return p;
return Vec3();
};*/