#include "geom.h"

void Matrix34Multiply_OnlySetOrigin( float *a, float *b, float *out ) 
{
	out[ 3] = a[0] * b[3] + a[1] * b[7] + a[ 2] * b[11] + a[ 3];
	out[ 7] = a[4] * b[3] + a[5] * b[7] + a[ 6] * b[11] + a[ 7];
	out[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11];
};

void Matrix3x3::convertquat(const Quat &q)
{
	float x = q.x, y = q.y, z = q.z, w = q.w,
		tx = 2*x, ty = 2*y, tz = 2*z,
		txx = tx*x, tyy = ty*y, tzz = tz*z,
		txy = tx*y, txz = tx*z, tyz = ty*z,
		twx = w*tx, twy = w*ty, twz = w*tz;
	a = Vec3(1 - (tyy + tzz), txy - twz, txz + twy);
	b = Vec3(txy + twz, 1 - (txx + tzz), tyz - twx);
	c = Vec3(txz - twy, tyz + twx, 1 - (txx + tyy));
}

float SqrPointRay(const Vec3 &Point, const Vec3 &ray_origin, const Vec3 &ray_direction, float* _pfParam)
{
	float DotMM;
	float T = 0.0f;
	Vec3 Diff;

	Diff = Point - ray_origin;
	T = Diff.x*ray_direction.x + Diff.y*ray_direction.y + Diff.z*ray_direction.z;//kDiff.Dot(ray_direction);

	if ( T <= 0.0f )
	{
		T = 0.0f;
	}
	else
	{
		DotMM = ray_direction.x*ray_direction.x + ray_direction.y*ray_direction.y + ray_direction.z*ray_direction.z;//.SquaredLength();
		if(DotMM)
		{
			T = T / DotMM;
			Diff -= ray_direction*T;
		}
	}

	if ( _pfParam )
		*_pfParam = T;

	return Diff.x*Diff.x + Diff.y*Diff.y + Diff.z*Diff.z;
}

bool SphereIntersect(const Vec3 &o, const Vec3 &d, const Vec3 &point, float r)
{
	float distsqr = SqrPointRay(point, o, d, 0);
	float radsqr = r*r;
	if (distsqr < radsqr)
		return true;
	else
		return false;
}

double AngleDifference(double a, double b)
{
	double dif = fmod(b - a + 3.1415926535895/*180*/, 2.0*3.1415926535895/*360*/);
	if (dif < 0)
		dif += 2.0*3.1415926535895;// 360;

	return dif - 3.1415926535895;// 180;
}