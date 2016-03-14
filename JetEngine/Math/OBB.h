
#ifndef OBB_HEADER
#define OBB_HEADER

#ifndef _WIN32
#include "Math/geom.h"
#include "Math/AABB.h"
#else
#include "geom.h"
#include "AABB.h"
#endif
class OBB
{

public:
	Vec3 pos;
	Vec3 r;
	Vec3 axes[3];

	OBB() {};
	OBB(const AABB& aabb, const Matrix4& transform)
	{
		this->SetFrom(aabb, transform);
	}

	void SetFrom(const AABB &aabb, const Matrix4& transform)
	{
		this->pos = transform*((aabb.min + aabb.max)/2.0f);
		Vec3 size = ((aabb.max - aabb.min) / 2);
		this->axes[0] = transform.GetBasisX();
		this->axes[1] = transform.GetBasisY();
		this->axes[2] = transform.GetBasisZ();
		this->r.x = size.x;
		this->r.y = size.y;
		this->r.z = size.z;
	};

	void ToTransformedAABB(AABB &aabb, Matrix4& transform)//need to redo sometime, terrible matrix handling
	{
		aabb.max = this->r;
		aabb.min = -this->r;

		transform = Matrix4(Matrix3x4(Vec4(this->axes[0],pos.x), Vec4(this->axes[1],pos.y), Vec4(this->axes[2],pos.z)));
		//todo matrix
	}

	bool Intersects(const Vec3& origin, const Vec3& direction)//slow as fuq, need to opimize sometime
	{
		Matrix4 mat;
		AABB aabb;
		this->ToTransformedAABB(aabb, mat);
		mat = mat.Inverse();

		return aabb.Intersects((Vec3)(mat*Vec4(origin,1)), (Vec3)(mat*Vec4(direction,0)));
	}

	bool Intersects(Plane* frustum)
	{
		for (int i = 0; i < 6; i++)
		{
			float x = this->axes[0].dot(Vec3(frustum[i].a, frustum[i].b, frustum[i].c)) >= 0.0f ? this->r.x*0.5f : this->r.x*-0.5f;
			float y = this->axes[1].dot(Vec3(frustum[i].a, frustum[i].b, frustum[i].c)) >= 0.0f ? this->r.y*0.5f : this->r.y*-0.5f;
			float z = this->axes[2].dot(Vec3(frustum[i].a, frustum[i].b, frustum[i].c)) >= 0.0f ? this->r.z-0.5f : this->r.z*-0.5f;

			const Vec3 diag = this->axes[0]*x + this->axes[1]*y + this->axes[2]*z;

			const Vec3 nPoint = this->pos - diag;

			if (nPoint.dot(Vec3(frustum[i].a, frustum[i].b, frustum[i].c)) + frustum[i].d >= 0.0f)
				return false;
		}
		return true;
	}
};
#endif