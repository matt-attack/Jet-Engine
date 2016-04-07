#pragma once



#include "CRenderer.h"
#include "../TerrainSystem.h"

class ObjModel;
struct TreeBillboard
{
	Vec3 position;
	Vec2 size;
	unsigned int color;
	float type;
};
class FoliageRenderer
{
	CTexture* texture;
	CShader* shader;
	ID3D11Buffer* mDrawVB, *mStreamOutVB;

	TreeBillboard* data;

	VertexDeclaration vd;

	int num_billboards;

	//Vec2 dimensions;

	int hack = 0;

	//things to render, updated every frame
	//std::vector<Vec3> trees;

	struct TreeModel
	{
		Vec2 dimensions;
		ObjModel* model;
	};
	std::vector<TreeModel> tree_models;

public:
	FoliageRenderer();
	~FoliageRenderer();

	void Init(HeightmapTerrainSystem* system);

	void GenerateImpostors();

	void Render(CRenderer* renderer, const CCamera& cam);

	void AddModel(const char* name);

	Vec2 GetImpostorSize(ObjModel* model);
};

