#ifndef VECTOR3_HEADER
#define VECTOR3_HEADER
#include <cmath>
#include <stdlib.h>

struct Vec4;
struct Vec3d;
class Matrix4f;


struct Viewport
{
	int Height, Width;
	int X, Y;
	float MinZ, MaxZ;
};

struct Vec2
{
	float x, y;
	Vec2() {}
	Vec2(float x, float y) : x(x), y(y) {};

	Vec2 operator+(const Vec2 &o) const { return Vec2(x + o.x, y + o.y); }
	Vec2 operator/(float k) const { return Vec2(x / k, y / k); }

};

struct Vec3
{
	union
	{
		struct { float x, y, z; };
		float v[3];
		/*union
		{
			Vec2 xy;
		}; */
	};

	Vec3() {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {};
	explicit Vec3(const float *v) : x(v[0]), y(v[1]), z(v[2]) {};
	explicit Vec3(const Vec4 &v);// : x(v.x), y(v.y), z(v.z) {};
	explicit Vec3(const Vec3d &v);

	float &operator[](int i) { return v[i]; }
	float operator[](int i) const { return v[i]; }

	bool operator==(const Vec3 &o) const { return x == o.x && y == o.y && z == o.z; }
	bool operator!=(const Vec3 &o) const { return x != o.x || y != o.y || z != o.z; }

	Vec3 operator+(const Vec3 &o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
	Vec3 operator-(const Vec3 &o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
	Vec3 operator+(float k) const { return Vec3(x+k, y+k, z+k); }
	Vec3 operator-(float k) const { return Vec3(x-k, y-k, z-k); }
	Vec3 operator-() const { return Vec3(-x, -y, -z); }
	Vec3 operator*(const Vec3 &o) const { return Vec3(x*o.x, y*o.y, z*o.z); }
	Vec3 operator/(const Vec3 &o) const { return Vec3(x/o.x, y/o.y, z/o.z); }
	Vec3 operator*(float k) const { return Vec3(x*k, y*k, z*k); }
	Vec3 operator/(float k) const { return Vec3(x/k, y/k, z/k); }

	Vec3 &operator+=(const Vec3 &o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vec3 &operator-=(const Vec3 &o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vec3 &operator+=(float k) { x += k; y += k; z += k; return *this; }
	Vec3 &operator-=(float k) { x -= k; y -= k; z -= k; return *this; }
	Vec3 &operator*=(const Vec3 &o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	Vec3 &operator/=(const Vec3 &o) { x /= o.x; y /= o.y; z /= o.z; return *this; }
	Vec3 &operator*=(float k) { x *= k; y *= k; z *= k; return *this; }
	Vec3 &operator/=(float k) { x /= k; y /= k; z /= k; return *this; }

	float dot(const Vec3 &o) const { return x*o.x + y*o.y + z*o.z; }
	float magnitude() const { return sqrtf(dot(*this)); }
	float length() const { return sqrtf(dot(*this)); }
	float squaredlen() const { return dot(*this); }
	float dist(const Vec3 &o) const { return (*this - o).magnitude(); }
	float distsqr(const Vec3 &o) const { return (*this - o).squaredlen(); }
	Vec3 getnormal() const 
	{ 
		float mag = magnitude();
		if (mag < 0.01)
			mag = 0.1f;
		return *this * (1.0f / mag); 
	}
	void normalize() {
		float mag = magnitude();
		if (mag < 0.01)
			mag = 0.1f;
		(*this) *= (1.0f / mag); 
	}

	void floor()
	{
		x = ::floor(x);
		y = ::floor(y);
		z = ::floor(z);
	}
	//void abs();// { x = ::abs(x); y = ::abs(y); z = ::abs(z); }

	Vec3 cross(const Vec3 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
	Vec3 reflect(const Vec3 &n) const { return *this - n*2.0f*dot(n); }
	Vec3 project(const Vec3 &n) const { return *this - n*dot(n); }
	float angle(const Vec3 &n) const { return acos(this->dot(n)); }

	static Vec3 random(const float dx, const float dy, const float dz)
	{
		return Vec3(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/dx)),
					static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/dy)),
					static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/dz)));//rand()%RAND_MAX, rand());
	}

	Vec3 toscreen(const Vec3 *pv, const Viewport *pviewport, const Matrix4f *pprojection, const Matrix4f *pview, const Matrix4f *pworld);
};

struct Vec3i
{
	union
	{
		struct { int x, y, z; };
		int v[3];
	};

	Vec3i() {}
	Vec3i(int x, int y, int z) : x(x), y(y), z(z) {};
	explicit Vec3i(const int *v) : x(v[0]), y(v[1]), z(v[2]) {};
	explicit Vec3i(const Vec4 &v);// : x(v.x), y(v.y), z(v.z) {};
	explicit Vec3i(const Vec3d &v);

	int &operator[](int i) { return v[i]; }
	int operator[](int i) const { return v[i]; }

	bool operator==(const Vec3i &o) const { return x == o.x && y == o.y && z == o.z; }
	bool operator!=(const Vec3i &o) const { return x != o.x || y != o.y || z != o.z; }

	Vec3i operator+(const Vec3i &o) const { return Vec3i(x+o.x, y+o.y, z+o.z); }
	Vec3i operator-(const Vec3i &o) const { return Vec3i(x-o.x, y-o.y, z-o.z); }
	Vec3i operator+(int k) const { return Vec3i(x+k, y+k, z+k); }
	Vec3i operator-(int k) const { return Vec3i(x-k, y-k, z-k); }
	Vec3i operator-() const { return Vec3i(-x, -y, -z); }
	Vec3i operator*(const Vec3i &o) const { return Vec3i(x*o.x, y*o.y, z*o.z); }
	Vec3i operator/(const Vec3i &o) const { return Vec3i(x/o.x, y/o.y, z/o.z); }
	Vec3i operator*(int k) const { return Vec3i(x*k, y*k, z*k); }
	Vec3i operator/(int k) const { return Vec3i(x/k, y/k, z/k); }

	Vec3i &operator+=(const Vec3i &o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vec3i &operator-=(const Vec3i &o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vec3i &operator+=(int k) { x += k; y += k; z += k; return *this; }
	Vec3i &operator-=(int k) { x -= k; y -= k; z -= k; return *this; }
	Vec3i &operator*=(const Vec3i &o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	Vec3i &operator/=(const Vec3i &o) { x /= o.x; y /= o.y; z /= o.z; return *this; }
	Vec3i &operator*=(int k) { x *= k; y *= k; z *= k; return *this; }
	Vec3i &operator/=(int k) { x /= k; y /= k; z /= k; return *this; }

	int dot(const Vec3i &o) const { return x*o.x + y*o.y + z*o.z; }
	float magnitude() const { return sqrtf(dot(*this)); }
	int length() const { return sqrtf(dot(*this)); }
	int squaredlen() const { return dot(*this); }
	int dist(const Vec3i &o) const { return (*this - o).magnitude(); }
	int distsqr(const Vec3i &o) const { return (*this - o).squaredlen(); }
	Vec3i getnormal() const 
	{ 
		float mag = magnitude();
		if (mag < 0.01)
			mag = 0.1f;
		return *this * (1.0f / mag); 
	}
	void normalize() {
		float mag = magnitude();
		if (mag < 0.01)
			mag = 0.1f;
		(*this) *= (1.0f / mag); 
	}
	Vec3i cross(const Vec3i &o) const { return Vec3i(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
	Vec3i reflect(const Vec3i &n) const { return *this - n*2.0f*dot(n); }
	Vec3i project(const Vec3i &n) const { return *this - n*dot(n); }
	//float angle(const Vec3i &n) const { return acos(this->dot(n)); }

	Vec3 toscreen(const Vec3 *pv, const Viewport *pviewport, const Matrix4f *pprojection, const Matrix4f *pview, const Matrix4f *pworld);
};


struct Vec3d
{
	union
	{
		struct { double x, y, z; };
		double v[3];
	};

	Vec3d() {}
	Vec3d(double x, double y, double z) : x(x), y(y), z(z) {}
	explicit Vec3d(const double *v) : x(v[0]), y(v[1]), z(v[2]) {}
	explicit Vec3d(const Vec3 &v) : x(v.x), y(v.y), z(v.z) {};

	double &operator[](int i) { return v[i]; }
	double operator[](int i) const { return v[i]; }

	bool operator==(const Vec3d &o) const { return x == o.x && y == o.y && z == o.z; }
	bool operator!=(const Vec3d &o) const { return x != o.x || y != o.y || z != o.z; }

	Vec3d operator+(const Vec3d &o) const { return Vec3d(x+o.x, y+o.y, z+o.z); }
	Vec3d operator-(const Vec3d &o) const { return Vec3d(x-o.x, y-o.y, z-o.z); }
	Vec3d operator+(double k) const { return Vec3d(x+k, y+k, z+k); }
	Vec3d operator-(double k) const { return Vec3d(x-k, y-k, z-k); }
	Vec3d operator-() const { return Vec3d(-x, -y, -z); }
	Vec3d operator*(const Vec3d &o) const { return Vec3d(x*o.x, y*o.y, z*o.z); }
	Vec3d operator/(const Vec3d &o) const { return Vec3d(x/o.x, y/o.y, z/o.z); }
	Vec3d operator*(double k) const { return Vec3d(x*k, y*k, z*k); }
	Vec3d operator/(double k) const { return Vec3d(x/k, y/k, z/k); }

	Vec3d &operator+=(const Vec3d &o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vec3d &operator-=(const Vec3d &o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vec3d &operator+=(double k) { x += k; y += k; z += k; return *this; }
	Vec3d &operator-=(double k) { x -= k; y -= k; z -= k; return *this; }
	Vec3d &operator*=(const Vec3d &o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	Vec3d &operator/=(const Vec3d &o) { x /= o.x; y /= o.y; z /= o.z; return *this; }
	Vec3d &operator*=(double k) { x *= k; y *= k; z *= k; return *this; }
	Vec3d &operator/=(double k) { x /= k; y /= k; z /= k; return *this; }

	double dot(const Vec3d &o) const { return x*o.x + y*o.y + z*o.z; }
	double magnitude() const { return sqrt(dot(*this)); }
	double length() const { return sqrt(dot(*this)); }
	double squaredlen() const { return dot(*this); }
	double dist(const Vec3d &o) const { return (*this - o).magnitude(); }
	double distsqr(const Vec3d &o) const { return (*this - o).squaredlen(); }
	Vec3d getnormal() const { return *this * (1.0f / magnitude()); }
	void normalize() { (*this) *= (1.0f / magnitude()); }
	Vec3d cross(const Vec3d &o) const { return Vec3d(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
	Vec3d reflect(const Vec3d &n) const { return *this - n*2.0f*dot(n); }
	Vec3d project(const Vec3d &n) const { return *this - n*dot(n); }
};

struct Vec4
{
	union
	{
		struct { float x, y, z, w; };
		struct { float a, r, g, b; };
#ifdef _WIN32
		struct { float a; Vec3 rbg; };
		struct { Vec3 xyz; float w; };
#endif
		float v[4];
	};

	Vec4() {}
	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	explicit Vec4(const Vec3 &p, float w = 0) : x(p.x), y(p.y), z(p.z), w(w) {}
	explicit Vec4(const float *v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

	float &operator[](int i) { return v[i]; }
	float operator[](int i) const { return v[i]; }

	bool operator==(const Vec4 &o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	bool operator!=(const Vec4 &o) const { return x != o.x || y != o.y || z != o.z || w != o.w; }

	Vec4 operator+(const Vec4 &o) const { return Vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
	Vec4 operator-(const Vec4 &o) const { return Vec4(x-o.x, y-o.y, z-o.z, w-o.w); }
	Vec4 operator+(float k) const { return Vec4(x+k, y+k, z+k, w+k); }
	Vec4 operator-(float k) const { return Vec4(x-k, y-k, z-k, w-k); }
	Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
	Vec4 neg3() const { return Vec4(-x, -y, -z, w); }
	Vec4 operator*(float k) const { return Vec4(x*k, y*k, z*k, w*k); }
	Vec4 operator/(float k) const { return Vec4(x/k, y/k, z/k, w/k); }
	Vec4 addw(float f) const { return Vec4(x, y, z, w + f); }

	Vec4 &operator+=(const Vec4 &o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
	Vec4 &operator-=(const Vec4 &o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
	Vec4 &operator+=(float k) { x += k; y += k; z += k; w += k; return *this; }
	Vec4 &operator-=(float k) { x -= k; y -= k; z -= k; w -= k; return *this; }
	Vec4 &operator*=(float k) { x *= k; y *= k; z *= k; w *= k; return *this; }
	Vec4 &operator/=(float k) { x /= k; y /= k; z /= k; w /= k; return *this; }

	float dot3(const Vec4 &o) const { return x*o.x + y*o.y + z*o.z; }
	float dot3(const Vec3 &o) const { return x*o.x + y*o.y + z*o.z; }
	float dot(const Vec4 &o) const { return dot3(o) + w*o.w; }
	float dot(const Vec3 &o) const { return x*o.x + y*o.y + z*o.z + w; }
	float magnitude() const { return sqrtf(dot(*this)); }
	float magnitude3() const { return sqrtf(dot3(*this)); }
	Vec4 getnormal() const { return *this * (1.0f / magnitude()); }
	void normalize() { (*this) *= (1.0f/magnitude()); }
	Vec3 cross3(const Vec4 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
	Vec3 cross3(const Vec3 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
};
#endif