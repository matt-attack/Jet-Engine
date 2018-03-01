
#ifndef AABB_HEADER
#define AABB_HEADER

#include <cstdlib>
#include "geom.h"

class AABB
{
public:
	Vec3 min;
	Vec3 max;

	AABB() {};
	AABB(const Vec3& _min, const Vec3& _max)
	{
		this->min = _min;
		this->max = _max;
	};

	AABB(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
	{
		this->min.x = xmin;
		this->min.y = ymin;
		this->min.z = zmin;
		this->max.x = xmax;
		this->max.y = ymax;
		this->max.z = zmax;
	};

	Vec3 Size()
	{
		return this->max - this->min;
	}

	bool Intersects(const AABB& other)
	{
		if (this->max.x < other.min.x ||
			this->max.y < other.min.y ||
			this->max.z < other.min.z ||
			this->min.x > other.max.x ||
			this->min.y > other.max.y ||
			this->min.z > other.max.z)
		{
			return false;
		}
		return true;
	};

	bool IntersectsPoint(const Vec3& p)
	{
		if (p.x > max.x || p.x < min.x ||
			p.y > max.y || p.y < min.y ||
			p.z > max.z || p.z < min.z)
		{
			return false;
		}
		return true;
	};

	bool Intersects(const Vec3& origin, const Vec3& direction/*, float &min_t, float &max_t*/) const
	{
		float t0 = 0;//Vec3(0);
		float t1 = 10000;//r.maxdistance;

		for (int i = 0; i < 3; ++i)
		{
			float invRayDir = 1.0f / direction[i];
			float near_t = (this->min[i] - origin[i]) * invRayDir;
			float far_t = (this->max[i] - origin[i]) * invRayDir;

			if(near_t > far_t)
			{

				float temp = near_t;

				near_t = far_t;
				far_t = temp;
			}

			t0 = near_t > t0 ? near_t : t0;
			t1 = far_t < t1 ? far_t : t1;

			if(t0 > t1) 
				return false;
		}

		//min_t = t0;
		//max_t = t1;

		return true;
	}

	Vec3 MinimumTranslationB(const AABB& other)
	{
		Vec3 amin = this->min;
		Vec3 amax = this->max;
		Vec3 bmin = other.min;
		Vec3 bmax = other.max;

		Vec3 mtd;

		float left = (bmin.x - amax.x);
		float right = (bmax.x - amin.x);

		float top = (bmin.y - amax.y);
		float bottom = (bmax.y - amin.y);

		float front = (bmin.z - amax.z);
		float back = (bmax.z - amin.z);

		// box dont intersect
		//if (left > 0 || right < 0) throw new Exception("no intersection");
		//if (top > 0 || bottom < 0) throw new Exception("no intersection");

		// box intersect. work out the mtd on both x and y axes.
		if (abs(left) < right)
			mtd.x = left;
		else
			mtd.x = right;

		if (abs(top) < bottom)
			mtd.y = top;
		else
			mtd.y = bottom;

		if (abs(front) < back)
			mtd.z = front;
		else
			mtd.z = back;

		// 0 all but the axis with the smallest mtd value.
		/*if (abs(mtd.x) < abs(mtd.y))
		{ //ok y > x
		if (abs(mtd.y) < abs(mtd.z))//check if z > or < than y
		{ // z > y so z > y > x
		mtd.z = 0;//z is biggest, so zero
		}
		else
		{ //y > z and y > x
		mtd.y = 0;//y is biggest, so zero

		//now determine if to zero x or z
		if (abs(mtd.x) < abs(mtd.z))
		mtd.z = 0;// z > x so zero the biggest one
		else
		mtd.x = 0;// x > z so zero the biggest one
		}
		mtd.y = 0;//zero y because its not the smallest
		}
		else//ok, x > y
		{// x > y
		if (abs(mtd.x) < abs(mtd.z))
		{ //z > x so z > x > y
		mtd.z = 0;//z is biggest, so zero
		}
		else
		{ //x > z and x > y
		mtd.x = 0;//x is biggest, so zero

		//now to determine if to zero y or z
		if (abs(mtd.y) < abs(mtd.z))
		mtd.z = 0;// z > y so zero the biggest one
		else
		mtd.y = 0;// y > z so zero the biggest one
		}
		mtd.x = 0;//zero x because its not the smallest
		}*/

		/*mtd.y = 0;
		if (abs(mtd.x) < abs(mtd.z))
		mtd.z = 0;
		else
		mtd.x = 0; */
		return mtd;
	};

	AABB &operator+=(const Vec3 &o) { min += o; max += o; return *this; }

	AABB &operator*=(const float &s) { min *= s; max *= s; return *this; }

	void FitPoint(const Vec3& point)
	{
		if (this->max.x < point.x)
			this->max.x = point.x;
		if (this->max.y < point.y)
			this->max.y = point.y;
		if (this->max.z < point.z)
			this->max.z = point.z;

		if (this->min.x > point.x)
			this->min.x = point.x;
		if (this->min.y > point.y)
			this->min.y = point.y;
		if (this->min.z > point.z)
			this->min.z = point.z;
	}

	void FitAABB(const AABB& b)
	{
		// Ignore zero-size boxes
		if( min == max )
		{
			min = b.min;
			max = b.max;
		}
		else if( b.min != b.max )
		{
			if( b.min.x < min.x ) { min.x = b.min.x; }
			if( b.min.y < min.y ) { min.y = b.min.y; }
			if( b.min.z < min.z ) { min.z = b.min.z; }
			if( b.max.x > max.x ) { max.x = b.max.x; }
			if( b.max.y > max.y ) { max.y = b.max.y; }
			if( b.max.z > max.z ) { max.z = b.max.z; }
		}
	}

	void Transform( const Matrix4 &m )
	{
		// Efficient algorithm for transforming an AABB, taken from Graphics Gems
		float minA[3] = { min.x, min.y, min.z }, minB[3];
		float maxA[3] = { max.x, max.y, max.z }, maxB[3];
		for( unsigned int i = 0; i < 3; ++i )
		{
			minB[i] = m.m[3][i];
			maxB[i] = m.m[3][i];
			for( unsigned int j = 0; j < 3; ++j )
			{
				float x = minA[j] * m.m[j][i];
				float y = maxA[j] * m.m[j][i];
				minB[i] += x < y ? x : y;//std::min( x, y );
				maxB[i] += x > y ? x : y;//std::max( x, y );
			}
		}
		min = Vec3( minB[0], minB[1], minB[2] );
		max = Vec3( maxB[0], maxB[1], maxB[2] );
	}

	Vec3 MinimumTranslation(const AABB& other)
	{
		Vec3 amin = this->min;
		Vec3 amax = this->max;
		Vec3 bmin = other.min;
		Vec3 bmax = other.max;

		Vec3 mtd;

		float left = (bmin.x - amax.x);
		float right = (bmax.x - amin.x);

		float top = (bmin.y - amax.y);
		float bottom = (bmax.y - amin.y);

		float front = (bmin.z - amax.z);
		float back = (bmax.z - amin.z);

		// box dont intersect
		//if (left > 0 || right < 0) throw new Exception("no intersection");
		//if (top > 0 || bottom < 0) throw new Exception("no intersection");

		// box intersect. work out the mtd on both x and y axes.
		if (abs(left) < right)
			mtd.x = left;
		else
			mtd.x = right;

		if (abs(top) < bottom)
			mtd.y = top;
		else
			mtd.y = bottom;

		if (abs(front) < back)
			mtd.z = front;
		else
			mtd.z = back;

		// 0 all but the axis with the smallest mtd value.
		if (abs(mtd.x) < abs(mtd.y))
		{ //ok y > x
			if (abs(mtd.y) < abs(mtd.z))//check if z > or < than y
			{ // z > y so z > y > x
				mtd.z = 0;//z is biggest, so zero
			}
			else
			{ //y > z and y > x
				mtd.y = 0;//y is biggest, so zero

				//now determine if to zero x or z
				if (abs(mtd.x) < abs(mtd.z))
					mtd.z = 0;// z > x so zero the biggest one
				else
					mtd.x = 0;// x > z so zero the biggest one
			}
			mtd.y = 0;//zero y because its not the smallest
		}
		else//ok, x > y
		{// x > y
			if (abs(mtd.x) < abs(mtd.z))
			{ //z > x so z > x > y
				mtd.z = 0;//z is biggest, so zero
			}
			else
			{ //x > z and x > y
				mtd.x = 0;//x is biggest, so zero

				//now to determine if to zero y or z
				if (abs(mtd.y) < abs(mtd.z))
					mtd.z = 0;// z > y so zero the biggest one
				else
					mtd.y = 0;// y > z so zero the biggest one
			}
			mtd.x = 0;//zero x because its not the smallest
		}

		/*mtd.y = 0;
		if (abs(mtd.x) < abs(mtd.z))
		mtd.z = 0;
		else
		mtd.x = 0; */
		return mtd;
	}
};

#define mmax(a,b)            (((a) > (b)) ? (a) : (b))

#define mmin(a,b)            (((a) < (b)) ? (a) : (b))

#define VECTOR Vec3
#define SCALAR float
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
	);
#endif
