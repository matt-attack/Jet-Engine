#pragma once

#include "CRenderer.h"
#include "../TerrainSystem.h"

class ObjModel;
struct TreeBillboard
{
	Vec3 position;
	Vec3 normal;
	Vec2 size;
	unsigned int color;
	float type;
};
class FoliageRenderer
{
	CTexture* texture;
	CShader* shader;
	CShader* shader_shadow;
	
	VertexDeclaration vd;

	int hack = 0;

	struct TreeModel
	{
		Vec2 dimensions;
		ObjModel* model;
	};
	std::vector<TreeModel> tree_models;
	std::vector<ObjModel*> render_models[8];//stupid hack


	struct TreeTile
	{
		int x, y;
		int size;
		CVertexBuffer vb;
		std::vector<TreeBillboard> data;
	};

	int size = 0;
	int tiles_dim = 0;
	TreeTile* tiles = 0;

public:
	FoliageRenderer();
	~FoliageRenderer();

	void Init(HeightmapTerrainSystem* system);

	void GenerateImpostors();

	void Render(CRenderer* renderer, const CCamera& cam);

	void RenderImpostors(CRenderer* renderer, const CCamera& cam);

	void AddModel(const char* name);

	TreeBillboard* AddTree(float x, float z);

	Vec2 GetImpostorSize(ObjModel* model);
};

