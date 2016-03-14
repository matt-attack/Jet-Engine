#pragma once

#include "../Math/Vector.h"
#include "CVertexBuffer.h"
#include "CIndexBuffer.h"


const int trail_size = 400;
const int num_trails = 40;

struct Trail
{
	bool used;
};

struct TailVertex
{
	Vec3 position, tangent;
	Vec2 uv;
	float width;
};

class TrailRenderer
{
	Trail trails[num_trails];
	CVertexBuffer vb;
	CIndexBuffer ib;

public:
	TrailRenderer();
	~TrailRenderer();

	int AllocateTrail();
	void FreeTrail(int id);

	void AppendToTrail(int id, const Vec3& position, const Vec3& tangent);

	void Render();
};

