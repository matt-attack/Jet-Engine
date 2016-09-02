#pragma once

#include "TerrainPatch.h"

class CCamera;
class ITerrainSystem
{
public:
	ITerrainSystem(void);
	virtual ~ITerrainSystem(void);

	virtual void Render(CCamera* cam, int player) = 0;
};

class VoxelTerrainSystem: public ITerrainSystem
{
public:
};

#include <map>
struct XYP
{
	int x, y;

	bool operator < (const XYP& o) const 
	{
		return (x < o.x || (x == o.x && y < o.y));
	}
	XYP() {}
	XYP(int x, int y) : x(x), y(y) {};
};
class TerrainMaterial;
class QuadTree;
class HeightmapTerrainSystem : public ITerrainSystem, Renderable
{
	int temp_player;//used for when waiting to be rendered to

	int world_size;//heightmap size
	int patch_size;//patchsize
	QuadTree** grid[2];
	ID3D11SamplerState* sampler, *textureSampler;

	ID3D11Texture2D* hmapt;
	ID3D11ShaderResourceView* hmapv;

	CShader *shader, *shader_s;

	VertexDeclaration vertex_declaration;

	//ok, lets have a LOD Tree
	float* heights;

	//downsampled heights for ray tracing through (4x less dense)
	float* heights_ds;

	bool done = false;
	bool need_to_reload_normals;

	TerrainMaterial* my_material;
public:

	HeightmapTerrainSystem();
	~HeightmapTerrainSystem();

	CTexture* grass, *rock;
	CTexture* nmap;

	void Load();

	void SaveHeightmap(const char* file);

	void GenerateNormals();

	virtual void Render(CCamera* cam, int player);
	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue);

	void SetHeight(int x, int y, float z);

	void UpdateHeights();

	float GetRoughHeight(float x, float y);
	float GetHeight(float x, float y);
	float GetHeightAndNormal(float x, float y, Vec3& normal);
	float GetHeightAndVectors(float x, float y, Vec3& normal, Vec3& xtangent, Vec3& ytangent);
};

