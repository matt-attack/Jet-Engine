#ifndef DECAL_HEADER
#define DECAL_HEADER

#ifndef MATT_SERVER
#include "CRenderer.h"
#include "CVertexBuffer.h"
#include "CTexture.h"

#include <vector>
#include <map>

#include <D3D11.h>

struct vert
{
	Vec3 pos;
	COLOR color;
	float u, v;
};

class DecalManager
{
	CVertexBuffer vb;
	std::vector<vert> verts;//have a map of vectors? one for each material?
	int maxcount; int oldest;
	CShader* shader;
	CTexture* texture;

public:
	int count;
	DecalManager(int limit)
	{
		count = 0;
		maxcount = limit;
		oldest = 0;
		shader = renderer->CreateShader(4, "Shaders/model_diffuse.shdr");
		texture = resources.get_unsafe<CTexture>("decals.png");
	}

	~DecalManager()
	{
		if (texture)
			texture->Release();
	}

	void AddDecal(char* texture, Vec3 pos, Vec3 tangent, Vec3 normal);

	void Draw();

	void Clear()
	{
		this->oldest = 0;
		this->count = 0;
		this->verts.clear();
	}
};

#endif
#endif