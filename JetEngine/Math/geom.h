#ifndef __GEOM_H__
#define __GEOM_H__

#include "Vector.h"
#include "Matrix.h"

#include <cmath>
#include <utility>

class Matrix4f;
typedef Matrix4f Matrix4;

struct Vec3;
class Quaternion;
#define DEG2RAD 3.1415926535892384f/180.0f
#include "Quaternion.h"
typedef Quaternion Quat;

//used only in animation
void Matrix34Multiply_OnlySetOrigin( float *a, float *b, float *out );

struct Matrix3x3
{
	Vec3 a, b, c;

	Matrix3x3() {}
	Matrix3x3(const Vec3 &a, const Vec3 &b, const Vec3 &c) : a(a), b(b), c(c) {}
	explicit Matrix3x3(const Quat &q) { convertquat(q); }
	explicit Matrix3x3(const Quat &q, const Vec3 &scale)
	{
		convertquat(q);
		a *= scale;
		b *= scale;
		c *= scale;
	}

	void convertquat(const Quat &q);
	Matrix3x3 operator*(const Matrix3x3 &o) const
	{
		return Matrix3x3(
			o.a*a.x + o.b*a.y + o.c*a.z,
			o.a*b.x + o.b*b.y + o.c*b.z,
			o.a*c.x + o.b*c.y + o.c*c.z);
	}
	Matrix3x3 &operator*=(const Matrix3x3 &o) { return (*this = *this * o); }

	void transpose(const Matrix3x3 &o)
	{
		a = Vec3(o.a.x, o.b.x, o.c.x);
		b = Vec3(o.a.y, o.b.y, o.c.y);
		c = Vec3(o.a.z, o.b.z, o.c.z);
	}
	void transpose() { transpose(Matrix3x3(*this)); }

	Vec3 transform(const Vec3 &o) const { return Vec3(a.dot(o), b.dot(o), c.dot(o)); }
	Vec3 transposedtransform(const Vec3 &o) const { return a*o.x + b*o.y + c*o.z; }
};

struct Matrix3x4
{
	Vec4 a, b, c;

	Matrix3x4() {}
	Matrix3x4(Matrix4 &m)
	{
		this->a = m.GetColumn(0);
		this->b = m.GetColumn(1);
		this->c = m.GetColumn(2);
	}

	static Matrix3x4 Identity()
	{
		return Matrix3x4(Vec4(1,0,0,0), Vec4(0,1,0,0), Vec4(0,0,1,0));	
	};

	Matrix3x4(const Vec4 &a, const Vec4 &b, const Vec4 &c) : a(a), b(b), c(c) {}
	explicit Matrix3x4(const Matrix3x3 &rot, const Vec3 &trans)
		: a(Vec4(rot.a, trans.x)), b(Vec4(rot.b, trans.y)), c(Vec4(rot.c, trans.z))
	{
	}
	explicit Matrix3x4(const Quat &rot, const Vec3 &trans)
	{
		*this = Matrix3x4(Matrix3x3(rot), trans);
	}
	explicit Matrix3x4(const Quat &rot, const Vec3 &trans, const Vec3 &scale)
	{
		*this = Matrix3x4(Matrix3x3(rot, scale), trans);
	}
	explicit Matrix3x4(const Vec3 &trans) : a(Vec4(1,0,0,trans.x)), b(Vec4(0,1,0,trans.y)), c(Vec4(0,0,1,trans.z))
	{

	}

	Matrix3x4 operator*(float k) const { return Matrix3x4(*this) *= k; }
	Matrix3x4 &operator*=(float k)
	{
		a *= k;
		b *= k;
		c *= k;
		return *this;
	}

	Matrix3x4 operator+(const Matrix3x4 &o) const { return Matrix3x4(*this) += o; }
	Matrix3x4 &operator+=(const Matrix3x4 &o)
	{
		a += o.a;
		b += o.b;
		c += o.c;
		return *this;
	}

	void invert(const Matrix3x4 &o)
	{
		Matrix3x3 invrot(Vec3(o.a.x, o.b.x, o.c.x), Vec3(o.a.y, o.b.y, o.c.y), Vec3(o.a.z, o.b.z, o.c.z));
		invrot.a /= invrot.a.squaredlen();
		invrot.b /= invrot.b.squaredlen();
		invrot.c /= invrot.c.squaredlen();
		Vec3 trans(o.a.w, o.b.w, o.c.w);
		a = Vec4(invrot.a, -invrot.a.dot(trans));
		b = Vec4(invrot.b, -invrot.b.dot(trans));
		c = Vec4(invrot.c, -invrot.c.dot(trans));
	}
	void invert() { invert(Matrix3x4(*this)); }

	Matrix3x4 operator*(const Matrix3x4 &o) const
	{
		//transpose me!!
		/*return Matrix3x4(
			(o.a*a.x + o.b*a.y + o.c*a.z).addw(a.w),
			(o.a*b.x + o.b*b.y + o.c*b.z).addw(b.w),
			(o.a*c.x + o.b*c.y + o.c*c.z).addw(c.w));*/
		return Matrix3x4(
			(a*o.a.x + b*o.a.y + c*o.a.z).addw(o.a.w),
			(a*o.b.x + b*o.b.y + c*o.b.z).addw(o.b.w),
			(a*o.c.x + b*o.c.y + c*o.c.z).addw(o.c.w));
	}
	Matrix3x4 &operator*=(const Matrix3x4 &o) { return (*this = *this * o); }

	void SetTranslation(const Vec3 &v) { a.w = v.x; b.w = v.y; c.w = v.z; }
	Vec3 GetTranslation() const { return Vec3(a.w,b.w,c.w);}
	Vec3 transform(const Vec3 &o) const { return Vec3(a.dot(o), b.dot(o), c.dot(o)); }
	Vec3 transformnormal(const Vec3 &o) const { return Vec3(a.dot3(o), b.dot3(o), c.dot3(o)); }

	//ok, im transposed, because 4x4, so lets do this to get my axes
	Vec3 xAxis() const
	{
		return Vec3(a.x, b.x, c.x);
	}

	Vec3 yAxis() const
	{
		return Vec3(a.y, b.y, c.y);
	}

	Vec3 zAxis() const
	{
		return Vec3(a.z, b.z, c.z);
	}
};

int I (int row, int col);
typedef Matrix4f Matrix4;
class Plane
{
public:
	union
	{
		struct { float a, b, c, d; };
		float v[4];
	};

	Plane()
	{

	}


	Plane(const Vec3& point, const Vec3& normal)
	{
		this->a = normal.x;
		this->b = normal.y;
		this->c = normal.z;
		this->d = -(point.dot(normal));
		//this->Normalize();
	}

	Plane(Vec3 p1, Vec3 p2, Vec3 p3)
	{
		Vec3 n = (p2 - p1).cross((p3 - p1)).getnormal();
		Vec3 r0 = p1;

		this->a = n.x;
		this->b = n.y;
		this->c = n.z;
		this->d = -(r0.dot(n));

		this->Normalize();
	}

	/*Plane& operator= ( const Plane& p )
	{
	this->a = p.a;
	this->b = p.b;
	this->c = p.c;
	this->d = p.d;
	return *this;
	}*/

	~Plane()
	{

	}

	float &operator[](int i) { return v[i]; }
	float operator[](int i) const { return v[i]; }

	void Normalize()
	{
		float l = sqrt(a*a + b*b + c*c);
		this->a /= l;
		this->b /= l;
		this->c /= l;
		this->d /= l;
	}

	float Dot(Vec3 pos) const
	{
		return this->a*(pos.x) + this->b*(pos.y) + this->c*(pos.z) + this->d;
	}
};

float SqrPointRay(const Vec3 &Point, const Vec3 &ray_origin, const Vec3 &ray_direction, float* _pfParam=0);

bool SphereIntersect(const Vec3 &o, const Vec3 &d, const Vec3 &point, float r);


template <class T> bool IntersectLineOBB(
	const Vec3& O, // Line origin
	const Vec3& D, // Line direction
	const Vec3& C, // Box center
	const Vec3 A[], // Box axes
	const Vec3& e, // Box extents
	T t[], // On output, parametric points of intersection
	T epsilon) // Tolerance for parallel test

{
	int parallel = 0;
	bool found = false;

	T DA[3];
	T dA[3];

	Vec3 d = C - O;
	for (int i = 0; i < 3; ++i)
	{
		DA[i] = D.dot(A[i]);
		dA[i] = d.dot(A[i]);

		if (fabs(DA[i]) < epsilon)
			parallel |= 1 << i;
		else
		{
			T es = (DA[i] > (T)0.0) ? e[i] : -e[i];
			T invDA = (T)1.0 / DA[i];

			if (!found)
			{
				t[0] = (dA[i] - es) * invDA;

				t[1] = (dA[i] + es) * invDA;

				found = true;
			}
			else
			{
				T s = (dA[i] - es) * invDA;

				if (s > t[0])
					t[0] = s;

				s = (dA[i] + es) * invDA;

				if (s < t[1])
					t[1] = s;

				if (t[0] > t[1])
					return false;
			}
		}
	}

	if (parallel)
		for (int i = 0; i < 3; ++i)
			if (parallel & (1 << i))
				if (fabs(dA[i] - t[0] * DA[i]) > e[i] || fabs(dA[i] - t[1] * DA[i]) > e[i])
					return false;

	return true;
}

double AngleDifference(double a, double b);
#endif