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

	void AddModel(const char* name);

	TreeBillboard* AddTree(float x, float z);

	Vec2 GetImpostorSize(ObjModel* model);
};

