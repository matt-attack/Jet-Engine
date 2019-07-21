#pragma once

#include "TerrainPatch.h"
#include "Graphics/CTexture.h"

extern float LODboost;

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

class TerrainMaterial;
class QuadTree;
class HeightmapTerrainSystem : public ITerrainSystem, public Renderable
{
	int temp_player;//used for when waiting to be rendered to

public:
	CRenderTexture* indirection_texture;
	CRenderTexture* tile_texture;
private:

	int world_size;//heightmap size
	int patch_size;//patchsize
	QuadTree** grid[4];
	ID3D11SamplerState* sampler, *textureSampler;

	//dont use these, use a texture object
	CTexture hmap;

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

	CTexture* grass, *rock, *snow, *dirt;
	CTexture* road;
	CTexture* nmap;
	CTexture* noise;

	void Load(float terrain_scale);

	void LoadHeightmap(const char* file);
	void GenerateHeightmap();


	void SaveHeightmap(const char* file);

	int GetSize()
	{
		return this->world_size*TerrainScale;
	}

	void GenerateNormals();

	virtual void Render(CCamera* cam, int player);
	virtual void Render(const CCamera* cam, std::vector<RenderCommand>* queue);

	void SetHeight(int x, int y, float z);

	void UpdateHeights();

	float GetRoughHeight(float x, float y);
	float GetHeight(float x, float y);
	float GetHeightAndNormal(float x, float y, Vec3& normal);
	float GetHeightAndVectors(float x, float y, Vec3& normal, Vec3& xtangent, Vec3& ytangent);

	//y component is height above the terrain
	//returns the road segment id
	struct RoadPoint
	{
		Vec3 pos;
		int connection = -1;
		float width = 16;//todo, use this to indicate type, number of lanes
	};
	struct RoadData
	{
		RoadPoint* points;
		int size;
		AABB bounds;
	};
	std::vector<RoadData> roads;
	int AddRoad(RoadPoint* points, unsigned int count);

	//atlas management stuff
	std::vector<int> free_tiles;//absolutely free ones

	//this renders the tile in as well as fills in the indirection texture when we are done
	int RenderTile(const std::vector<HeightmapTerrainSystem::RoadData>& roads, int x, int y, int scale, int id = -1);//returns the tile number

	void MarkTileFreed(int num);

private:
	void SetupChunks();
};

