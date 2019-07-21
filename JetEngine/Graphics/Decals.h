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
	CVertexBuffer vb_;
	std::vector<vert> verts;//have a map of vectors? one for each material?
	int max_count_; int oldest_;
	CShader* shader_;
	CTexture* texture_;
	bool dirty_;
	std::mutex mutex_;

public:
	int count_;
	DecalManager(int limit) : dirty_(false), shader_(0), texture_(0), count_(0), max_count_(limit), oldest_(0)
	{
		//shader = resources.get_shader("Shaders/model_diffuse.shdr");//renderer->CreateShader(4, "Shaders/model_diffuse.shdr");
		//texture = resources.get_unsafe<CTexture>("decals.png");
	}

	~DecalManager()
	{
		if (texture_)
			texture_->Release();
	}

	void AddDecal(char* texture, Vec3 pos, Vec3 tangent, Vec3 normal);

	void Draw();

	void Clear()
	{
		oldest_ = 0;
		count_ = 0;
		this->verts.clear();
	}
};

#endif
#endif