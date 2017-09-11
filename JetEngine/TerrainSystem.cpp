#include "TerrainSystem.h"
#include "Util/Profile.h"
#include "camera.h"

#include "Graphics/CRenderer.h"
#include "Graphics/Renderable.h"
#include "Graphics/Renderer.h"
#include "Graphics/CTexture.h"
#include "Graphics/Shader.h"
#include "Graphics/RenderTexture.h"


#include "Util/Noise.h"

#include <fstream>
#include <ostream>

float LODboost = 4;//todo: break this out into an option for terrain quality


#include "IMaterial.h"
class TerrainMaterial : public IMaterial
{

public:
	ID3D11SamplerState* sampler, *textureSampler;
	CTexture *nmap, *grass, *rock;
	CTexture *indirection, *tiles;
	TerrainMaterial() : IMaterial("Terrain")
	{

	}

	virtual void Apply(CRenderer* renderer)
	{
		IMaterial::Apply(renderer);

		renderer->SetCullmode(CULL_CW);

		renderer->context->VSSetSamplers(0, 1, &this->sampler);
		renderer->context->PSSetSamplers(0, 1, &this->sampler);
		renderer->context->PSSetSamplers(5, 1, &this->textureSampler);
		renderer->context->PSSetShaderResources(0, 1, &nmap->texture_rv);
		//renderer->context->PSSetShaderResources(6, 1, &grass->texture);
		//renderer->context->PSSetShaderResources(7, 1, &rock->texture);

		//renderer->SetPrimitiveType(PT_TRIANGLELIST);
		renderer->SetCullmode(CULL_CW);

		renderer->EnableAlphaBlending(false);
		//renderer->context->PSSetSamplers(5, 1, &this->textureSampler);
		//renderer->context->PSSetSamplers(0, 1, &this->sampler);

		renderer->SetFilter(5, FilterMode::Point);

		//renderer->context->PSSetShaderResources(0, 1, &nmap->texture);

		renderer->context->PSSetShaderResources(6, 1, &this->indirection->texture_rv);
		renderer->context->PSSetShaderResources(7, 1, &this->tiles->texture_rv);
	}
};

static inline float GetDistanceToBox(float x, float y, float size,
	float vx, float vz)
{
	//assert(BoundBox.fMaxX >= BoundBox.fMinX && 
	//     BoundBox.fMaxY >= BoundBox.fMinY && 
	//    BoundBox.fMaxZ >= BoundBox.fMinZ);
	float fdX = (vx > x + size/*BoundBox.fMaxX*/) ? (vx - (x + size)/*BoundBox.fMaxX*/) : ((vx < x/*BoundBox.fMinX*/) ? (/*BoundBox.fMinX*/x - vx) : 0.f);
	//float fdY = (Pos.y > BoundBox.fMaxY) ? (Pos.y - BoundBox.fMaxY) : ( (Pos.y < BoundBox.fMinY) ? (BoundBox.fMinY - Pos.y) : 0.f );
	float fdZ = (vz > y + size/*BoundBox.fMaxZ*/) ? (vz - (y + size)/*BoundBox.fMaxZ*/) : ((vz < y/*BoundBox.fMinZ*/) ? (/*BoundBox.fMinZ*/y - vz) : 0.f);
	//.. assert(fdX >= 0 && fdY >= 0 && fdZ >= 0);

	// D3DXVECTOR3 RangeVec(fdX, fdY, fdZ);
	return sqrt(fdX*fdX + fdZ*fdZ);//D3DXVec3Length( &RangeVec );
}


//represents a terrain chunk
class QuadTreeNode
{
	friend class HeightmapTerrainSystem;
	TerrainPatch* patch;
	QuadTreeNode* parent;
	HeightmapTerrainSystem* root;
public:
	int x, y;
	int size;

	int tile_id = -1;

	AABB aabb;

	QuadTreeNode* northwest;
	QuadTreeNode* northeast;
	QuadTreeNode* southwest;
	QuadTreeNode* southeast;

	QuadTreeNode(HeightmapTerrainSystem* root, QuadTreeNode* parent, int x, int y, int size)
	{
		this->root = root;
		this->parent = parent;
		northeast = 0;
		northwest = 0;
		southeast = 0;
		southwest = 0;
		this->x = x;
		this->y = y;
		this->size = size;
		this->patch = 0;
		this->state = 0;

		this->aabb.min = Vec3(x, 0, y);
		this->aabb.max = Vec3(x + size*TerrainScale, 1000, y + size*TerrainScale);
	}

	~QuadTreeNode()
	{
		delete this->patch;
		delete this->northeast;
		delete this->northwest;
		delete this->southeast;
		delete this->southwest;

		if (tile_id >= 0)
			this->root->MarkTileFreed(this->tile_id);
	}
	//todo: thread terrain update
	//	also need to work on the subdivision of roads into lower tiles for rendering
	void Subdivide()
	{
		//512 is highest level which is 1/4th the size of the border
		//512 = 
		//todo: also subdivide roads

		//virtual texture is 64 tiles across
		northwest = new QuadTreeNode(root, this, x, y, size / 2);
		northeast = new QuadTreeNode(root, this, x + size*TerrainScale / 2, y, size / 2);
		southwest = new QuadTreeNode(root, this, x, y + size*TerrainScale / 2, size / 2);
		southeast = new QuadTreeNode(root, this, x + size*TerrainScale / 2, y + size*TerrainScale / 2, size / 2);

		//add all the roads
		for (int i = 0; i < this->roads.size(); i++)
		{
			this->northeast->AddRoad(i, &this->roads[i]);
			this->southeast->AddRoad(i, &this->roads[i]);
			this->northwest->AddRoad(i, &this->roads[i]);
			this->southwest->AddRoad(i, &this->roads[i]);
		}
	}

	void RegenVertices(float* data)
	{
		if (this->northeast)
		{
			this->northeast->RegenVertices(data);
			this->northwest->RegenVertices(data);
			this->southeast->RegenVertices(data);
			this->southwest->RegenVertices(data);
		}
		if (this->patch)
			this->patch->GenerateVertices(data);
	}

	std::vector<HeightmapTerrainSystem::RoadData> roads;
	void AddRoad(int id, HeightmapTerrainSystem::RoadData* road)
	{
		//check if we are in bounds
		if (!this->aabb.Intersects(road->bounds))
			return;

		//printf("hi im a road that needs to be added");

		//lets refine the road down (todo)

		this->roads.push_back(*road);

		if (this->northeast)
		{
			this->northeast->AddRoad(id, road);
			this->southeast->AddRoad(id, road);
			this->northwest->AddRoad(id, road);
			this->southwest->AddRoad(id, road);
		}
	}

	int GetLOD(CCamera* cam)
	{
		float dist = GetDistanceToBox(x, y, size*TerrainScale, cam->_pos.x, cam->_pos.z);// sqrt((cam->_pos.x - (this->x+this->size/2))*(cam->_pos.x - (this->x+this->size/2)) + (cam->_pos.z - (this->y+this->size/2))*(cam->_pos.z - (this->y+this->size/2)));
		//float d = (float)16/(float)size;
		//d /= dist;

		//ok, lets calculate the vertex density as a function of distance
		//higher lod = more dense
		//every 2 lod = 2x the density

		//so lets just try and halve the density every 32 meters
		float density = dist > 0 ? 24 /*maybe put a density control here*/ / dist : 200000/*infinity*/;//dist/32;
		//ok lets calculate required density = verts/size

		int v = density*size;// / TerrainScale;
		//ok translate v->lod

		//lod of 8 = 16 verts 6 = 8, 4 = 4, 2 = 2, 0 = 1
		//log2(verts)*2 = lod
		int lod = v;
		if (v >= 96)
			lod = 13;
		if (v >= 64)
			lod = 12;
		if (v >= 48)
			lod = 11;
		else if (v >= 32)
			lod = 10;
		else if (v >= 24)
			lod = 9;
		else if (v >= 16)
			lod = 8;
		else if (v >= 12)//dumb case
			lod = 7;
		else if (v >= 8)
			lod = 6;
		else if (v >= 6)//dumb case
			lod = 5;
		else
			lod = v;//simplification of last 5 cases

		//if (this->patch->maxy - this->patch->miny > 30)// && showdebug > 1)
		//lod = 8;
		/*else if (v >= 4)
		lod = 4;
		else if (v >= 3)//dumb case
		lod = 3;
		else if (v >= 2)
		lod = 2;
		else if (v >= 1)//dumb case
		lod = 1;
		else
		lod = 0;*/

		//if (this->patch->maxy - this->patch->miny > 30 && dist < 4000)//showdebug > 1)
		//lod += 4;
		//else if (this->patch->maxy - this->patch->miny > 30)
		//lod += 2;

		return lod + LODboost;
	}

	void Render(float* heights, CCamera* cam, int p = 0)
	{
		//frustum cull
		if (cam->BoxInFrustum(this->aabb) == false)
			return;

		//renderer->DrawBoundingBox(this->aabb.min, this->aabb.max);

		//recurse down if we can
		if (northeast)
		{
			northeast->Render(heights, cam, 1);
			northwest->Render(heights, cam, 0);
			southwest->Render(heights, cam, 2);
			southeast->Render(heights, cam, 3);
		}
		else
		{
			if (this->patch == 0)
			{
				//allocate
				this->patch = new TerrainPatch(x, y, size);
				this->patch->GenerateVertices(heights);
				this->aabb.min.y = this->patch->miny;
				this->aabb.max.y = this->patch->maxy;


				//this is bad, but oh well
				int lod = this->GetLOD(cam);
				/*bool xi = false, xd = false, yi = false, yd = false;
				switch (p)
				{
				case 0://northeast
				if (parent->northwest->northeast)
				xd = true;
				if (parent->southeast->northeast)
				yi = true;
				break;
				case 1://northwest
				if (parent->northeast->northeast)
				xi = true;
				if (parent->southwest->northeast)
				yd = true;
				break;
				case 2://southwest
				if (parent->northwest->northeast)
				xd = true;
				if (parent->southeast->northeast)
				yi = true;
				break;
				case 3://southeast
				if (parent->northeast->northeast)
				xi = true;
				if (parent->southwest->northeast)
				yd = true;
				break;
				}*/

				this->patch->GenerateIndices(lod, true, true, true, true);
			}
			//if (this->patch->level == 8)
			//return;
			//render myself
			this->patch->Render(renderer, 0);
		}
	}

	void Render(float* heights, CCamera* cam, std::vector<RenderCommand>* queue, int p = 0)
	{
		//frustum cull
		//if (cam->BoxInFrustum(this->aabb) == false)
		//	return;

		//renderer->DrawBoundingBox(this->aabb.min, this->aabb.max);

		//recurse down if we can
		if (northeast)
		{
			northeast->Render(heights, cam, queue, 1);
			northwest->Render(heights, cam, queue, 0);
			southwest->Render(heights, cam, queue, 2);
			southeast->Render(heights, cam, queue, 3);
		}
		else
		{
			if (this->patch == 0)
			{
				//allocate
				this->patch = new TerrainPatch(x, y, size);
				this->patch->GenerateVertices(heights);
				this->aabb.min.y = this->patch->miny;
				this->aabb.max.y = this->patch->maxy;

				this->tile_id = this->root->RenderTile(this->roads, x / (TexturePatchSize * TerrainScale), y / (TexturePatchSize * TerrainScale), size / TexturePatchSize);

				//this is bad, but oh well
				int lod = this->GetLOD(cam);

				this->patch->GenerateIndices(lod, true, true, true, true);
			}

			//if (this->patch->level == 8)
			//return;
			//render myself
			this->patch->Render(renderer, 0, queue, root);
		}
	}

	//forms a lod around this position by performing an insertion
	void SetLOD(int x, int y)
	{
		if (this->northeast == 0)
		{
			//it belongs in me, subdivide if we can
			if (this->size <= PatchSize)//16)
				return;

			this->Subdivide();
		}

		if (x < this->x + size)
		{
			//west
			if (y < this->y + size)
				northwest->SetLOD(x, y);
			else
				southwest->SetLOD(x, y);
		}
		else
		{
			//east
			if (y < this->y + size)
				northeast->SetLOD(x, y);
			else
				southeast->SetLOD(x, y);
		}
	}

	char state;
	void UpdateLOD(CCamera* cam)
	{
		//todo decouple texture lod and this lod
		if (this->northeast)
		{
			this->northeast->UpdateLOD(cam);
			this->northwest->UpdateLOD(cam);
			this->southeast->UpdateLOD(cam);
			this->southwest->UpdateLOD(cam);

			if (state == -1)//if I need to reduce LOD by unsplitting
			{
				//remove children
				delete this->northeast;
				delete this->northwest;
				delete this->southwest;
				delete this->southeast;

				if (this->tile_id >= 0)
					this->root->RenderTile(this->roads, x / (TexturePatchSize * TerrainScale), y / (TexturePatchSize * TerrainScale), size / TexturePatchSize, this->tile_id);


				this->northeast = this->northwest = this->southeast = this->southwest = 0;
				state = 0;
			}
		}
		else
		{
			//im a leaf!
			//check if I need to be increased or decreased
			if (this->patch)
			{
				//this is a rough metric of vertex/distance density
				float dist = GetDistanceToBox(x, y, size*TerrainScale, cam->_pos.x, cam->_pos.z);// sqrt((cam->_pos.x - (this->x+this->size/2))*(cam->_pos.x - (this->x+this->size/2)) + (cam->_pos.z - (this->y+this->size/2))*(cam->_pos.z - (this->y+this->size/2)));

				//ok, lets calculate the vertex density as a function of distance
				//higher lod = more dense
				//every 2 lod = 2x the density

				//so lets just try and halve the density every 32 meters
				float density = dist > 0 ? 24/*12*/ / dist : 200000/*infinity*/;//dist/32;
				//ok lets calculate required density = verts/size

				//relative density needed
				float rd = density*size / TerrainScale;

				int v = rd;// density*size; / TerrainScale;
				//ok translate v->lod

				//9 = 32
				//lod of 8 = 16 verts 6 = 8, 4 = 4, 2 = 2, 0 = 1
				//log2(verts)*2 = lod
				int lod = v;
				//if (v >= 24)
				//lod = 9;
				if (v >= 48)
					lod = 11;
				else if (v >= 32)
					lod = 10;
				else if (v >= 24)
					lod = 9;
				else if (v >= 16)
					lod = 8;
				else if (v >= 12)//dumb case
					lod = 7;
				else if (v >= 8)
					lod = 6;
				else if (v >= 6)//dumb case
					lod = 5;
				else
					lod = v;//simplification of last 5 cases

				lod = this->GetLOD(cam);

				//ok, need to fix ambient light with shadows
				//	just calculate ambient term in the pixel shader if in shadow
				/*lets just do a light bounce shader
				gotta modify this to not be a patent infrigement
				float3 AmbientLight(const float3 worldNormal)
				{
				float3 nSquared = worldNormal * worldNormal;
				int3 isNegative = (worldNormal < 0.0);
				float3 linearColor;
				linearColor = nSquared.x * cAmbientCube[isNegative.x] +
				nSquared.y * cAmbientCube[isNegative.y + 2] +
				nSquared.z * cAmbientCube[isNegative.z + 4];
				return linearColor;
				}
				lets just use the ambient cube
				make me distance based*/

				//if (lod > PatchMaxLOD)
				//lod = PatchMaxLOD;

				//if cam is inside, lod = 8
				//if (this->size > 16 && lod >= 9)
				if (lod >= 11/*rd >= (PatchSize + PatchSize / 2)*/ && this->size > PatchSize)
					this->Subdivide();
				else if (this->parent && lod < PatchMaxLOD - 2)
				{
					//the lower this is, the smaller the vertex count, but the greater the drawcalls
					//1 is a good minimum
					//2 makes the distribution more uniform between visisted/nonvisited
					//4 is about perfect, lower frametime spend drawing
					//PatchSizePower + 1)//4)///density*size < 4)
					//check if parent will subdivide first
					if (this->parent->GetLOD(cam) < 11)
						this->parent->state = -1;
					else
					{
						//if (lod == 2)
						//need to figure out when to set the bools
						//this->patch->GenerateIndices(lod, false, false, false, false);
						//else

						/*if (this->parent)
						{
						lod = 7;

						if (this->parent->northwest == this)
						this->patch->GenerateIndices(lod, false, true, false, true);
						else if (this->parent->southeast == this)
						this->patch->GenerateIndices(lod, true, false, true, false);
						else if (this->parent->northeast == this)
						this->patch->GenerateIndices(lod, true, false, false, true);
						else
						this->patch->GenerateIndices(lod, false, true, true, false);
						}
						else*/
						this->patch->GenerateIndices(lod, true, true, true, true);
						//this->patch->GenerateIndices(lod, false, false, false, false);
					}
				}
			}
		}
	}
};

class QuadTree
{
	QuadTreeNode* root;

public:
	QuadTree(int x, int y, int size, HeightmapTerrainSystem* _root)
	{
		root = new QuadTreeNode(_root, 0, x, y, size);
		root->northeast = 0;
		root->northwest = 0;
		root->southeast = 0;
		root->southwest = 0;
	}

	~QuadTree()
	{
		delete this->root;
	}

	QuadTreeNode* Root()
	{
		return this->root;
	}
};

ITerrainSystem::ITerrainSystem(void)
{
}

ITerrainSystem::~ITerrainSystem(void)
{
}

HeightmapTerrainSystem::HeightmapTerrainSystem()
{
	this->castsShadows = false;

	this->matrix = Matrix4::Identity();

	grass = 0;
	hmap.texture = 0;
	hmap.texture_rv = 0;
	need_to_reload_normals = false;
	patch_size = 512;


	this->heights = 0;
}

HeightmapTerrainSystem::~HeightmapTerrainSystem()
{
	delete[] this->heights;
	this->sampler->Release();
	this->textureSampler->Release();
	//this->hmapv->Release();
	//this->hmapt->Release();

	this->grass->Release();
	this->rock->Release();

	for (int i = 0; i < 4; i++)
	{
		for (int x = 0; x < world_size / patch_size; x++)
		{
			for (int y = 0; y < world_size / patch_size; y++)
			{
				delete this->grid[i][x*(world_size / patch_size) + y];
			}
		}
		delete[] this->grid[i];
	}
}

std::function<void()> render;
int flipper = 0;
void HeightmapTerrainSystem::Render(CCamera* cam, std::vector<RenderCommand>* queue)
{
	//submit for each structure
	auto grid = this->grid[this->temp_player];
	for (int x = 0; x < world_size / patch_size; x++)
	{
		for (int y = 0; y < world_size / patch_size; y++)
		{
			grid[x*(world_size / patch_size) + y]->Root()->Render(this->heights, cam, queue);
		}
	}
}

void HeightmapTerrainSystem::Render(CCamera* cam, int player)
{
	PROFILE("Terrian Render");
	//GPUPROFILE("Terrain Render");

	this->temp_player = player;

	//generate normals if not done
	if (!done)
	{
		done = true;
		this->GenerateNormals();

		Viewport oldvp;
		renderer->GetViewport(&oldvp);
		auto ort = renderer->GetRenderTarget(0);


		//draw into the tile texture with tile numbers (and colors
		renderer->SetRenderTarget(0, this->tile_texture);
		Viewport vp;

		renderer->SetPixelTexture(0, 0);

		renderer->EnableAlphaBlending(false);

		const int tile_size = TextureAtlasTileSize;
		const int map_w = TextureAtlasSize;
		vp.Height = vp.Width = map_w;
		vp.X = vp.Y = 0;
		vp.MinZ = 0;
		vp.MaxZ = 1;
		renderer->SetViewport(&vp);
		srand(5);
		//drawrect needs to use the viewport size, not the backbuffer!
		this->tile_texture->Clear(1, 1, 1, 1);
		for (int x = 0; x < map_w / tile_size; x++)
		{
			for (int y = 0; y < map_w / tile_size; y++)
			{
				Rect r;
				r.bottom = tile_size * x;
				r.top = tile_size * (x + 1);
				r.right = tile_size * y;
				r.left = tile_size * (y + 1);
				renderer->DrawRect(&r, COLOR_ARGB(255, rand() % 256, rand() % 256, rand() % 256));
				std::string num = std::to_string(x * (map_w / tile_size) + y);
				renderer->DrawText(r.right + 50, r.bottom + 50, num.c_str(), COLOR_ARGB(255, 0, 0, 0));
			}
		}

		//setup the free tile list
		for (int i = 0; i < (TextureAtlasSize / TextureAtlasTileSize)*(TextureAtlasSize / TextureAtlasTileSize); i++)
			this->free_tiles.push_back(i);

		//now draw into indirection texture with random indices
		renderer->SetRenderTarget(0, this->indirection_texture);
		vp.Height = vp.Width = this->world_size / PatchSize;
		vp.X = vp.Y = 0;
		vp.MinZ = 0;
		vp.MaxZ = 1;
		renderer->SetViewport(&vp);
		this->indirection_texture->Clear(1, 0, 1, 1);

		//ok, lets start by clearing the texture to all use the L0 level (later we will optimize this to be better looking with one 2048x2048 texture or something)
		//this->indirection_texture->Clear(1, 16.0f/255.0f, 0, 0);

		//now request the highest level LOD
		this->RenderTile(this->roads, 0, 0, this->world_size / TexturePatchSize);

		//restore viewport
		renderer->SetViewport(&oldvp);

		renderer->SetRenderTarget(0, &ort);
	}

	if (this->need_to_reload_normals)
	{
		this->need_to_reload_normals = false;
		render();
	}

	this->my_material->nmap = this->nmap;


	r.AddRenderable(this);

	flipper++;
	//todo: later move the update outside of render
	auto grid = this->grid[player];
	for (int x = 0; x < world_size / patch_size; x++)
	{
		for (int y = 0; y < world_size / patch_size; y++)
		{
			if (flipper++ % 2)//only updates half each frame
				grid[x*(world_size / patch_size) + y]->Root()->UpdateLOD(cam);
		}
	}
	//this->RenderTile(0, 0, 64, 255);
}

int HeightmapTerrainSystem::RenderTile(const std::vector<HeightmapTerrainSystem::RoadData>& roads, int x, int y, int scale, int id)
{
	std::swap(x, y);

	//remove it from the list
	if (x%scale != 0)
		throw 7;
	if (y%scale != 0)
		throw 7;

	int num = id;
	if (id == -1)
	{
		if (this->free_tiles.size())
		{
			num = this->free_tiles.back();
			this->free_tiles.pop_back();
		}
		/*else if (this->unused_tiles.size())
		{
		num = this->unused_tiles.back();
		this->unused_tiles.pop_back();
		}*/
	}

	if (num == -1)
		return -1;//we couldnt find a free tile

	//ok, now render the terrain here
	Viewport oldvp, vp;
	renderer->GetViewport(&oldvp);
	auto ort = renderer->GetRenderTarget(0);

	renderer->SetPixelTexture(0, 0);

	renderer->EnableAlphaBlending(false);

	const int tile_size = TextureAtlasTileSize;
	const int map_w = TextureAtlasSize;
	const int tile_dim = map_w / tile_size;

	//if we already have the id, there is no need to re-render as the tile is already allocated
	//in this case we just update the indirection buffer
	//if (num != id)
	{
		//draw into the tile texture with tile numbers (and colors)
		renderer->SetRenderTarget(0, this->tile_texture);

		vp.Height = vp.Width = map_w;
		vp.X = vp.Y = 0;
		vp.MinZ = 0;
		vp.MaxZ = 1;
		renderer->SetViewport(&vp);

		renderer->SetShader(resources.get_unsafe<CShader>("Shaders/generate_tile.shdr"));

		struct data
		{
			Vec4 position;
			float scale;
		} td;
		td.position = Vec4(x, y, 0, 0);
		td.scale = scale;
		renderer->shader->cbuffers["Data"].UploadAndSet(&td, sizeof(data));


		//set textures
		renderer->SetPixelTexture(0, this->nmap);
		renderer->SetPixelTexture(1, this->rock);
		renderer->SetPixelTexture(2, this->grass);
		renderer->SetPixelTexture(3, this->snow);
		renderer->SetPixelTexture(4, this->hmap);
		renderer->SetPixelTexture(5, this->noise);

		renderer->SetFilter(0, FilterMode::Linear);

		//draw quad
		int tileno = num;
		int offx = (tileno / tile_dim);
		int offy = (tileno % tile_dim);

		Rect r;
		r.bottom = tile_size * offx;
		r.top = tile_size * (offx + 1);
		r.right = tile_size * offy;
		r.left = tile_size * (offy + 1);

		//lets use UV to encode where in the map we go
		//use r and g for offset, a for scale
		//and get world space pos in this shader
		//need to figure out whats wrong with this
		float umin = x * TexturePatchSize;
		float umax = (x + scale) * TexturePatchSize;
		std::swap(umin, umax);
		float vmin = y * TexturePatchSize;
		float vmax = (y + scale) * TexturePatchSize;
		std::swap(vmin, vmax);
		renderer->DrawRectUV(&r, Vec2(umin, vmin), Vec2(umin, vmax), Vec2(umax, vmin), Vec2(umax, vmax), COLOR_ARGB(255, 255, 255, 255), false);

		//draw it here

		struct TLVERTEX2
		{
			//float x;
			//float y;
			//float z;
			Vec3 pos;
			float rhw;
			COLOR color;
			float u;
			float v;
		};
		TLVERTEX2 vertices[6];

		//lay out vertices in a zig zag format
		//1 - 4 - 6
		//| \ | \ |
		//2 - 3 - 5

		renderer->SetPixelTexture(0, this->road);
		renderer->context->RSSetState(renderer->rs_scissor);
		D3D11_RECT r2;
		r2.bottom = r.top;
		r2.top = r.bottom;
		r2.left = r.right;
		r2.right = r.left;
		renderer->context->RSSetScissorRects(1, &r2);

		for (auto road : roads)
		{
			//render each road
			std::vector<Vec3> points;//points of the trail

			RoadPoint* source_points = road.points;

			//lets draw a road going through the map
			for (int i = 0; i < road.size; i++)
			{
				Vec3 p;
				p.x = source_points[i].pos.x;
				p.z = 0;
				p.y = source_points[i].pos.z;
				
				points.push_back(p);
			}

			//needs to be 2x as long
			//points.push_back(Vec3(0, 1024+32, 0));
			//points.push_back(Vec3(TexturePatchSize * TerrainScale, 1024+24, 0));
			//points.push_back(Vec3(TexturePatchSize * TerrainScale*200, 1024+32, 0));

			Vec3 tile_pos;
			tile_pos.z = 0;
			tile_pos.x = ((((float)r.right) / TextureAtlasSize)*2.0f) - 1.0f;
			tile_pos.y = 1.0f - ((((float)r.top) / TextureAtlasSize)*2.0f);

			Vec3 tile_world(y*(TexturePatchSize * TerrainScale), x*(TexturePatchSize * TerrainScale), 0);
			float tile_scale = ((2.0 / (TextureAtlasSize / TextureAtlasTileSize)) / scale) / (TexturePatchSize*TerrainScale);
			float road_size = 16;
			int i = 0;
			Vec3 tangent = Vec3(0, 1, 0);
			float v = 0;
			for (int p = 0; p < points.size(); p++)
			{
				if (p + 1 < points.size())
					tangent = (points[p] - points[p + 1]).cross(Vec3(0, 0, 1)).getnormal();
				
				Vec3 lp = points[p] - tile_world;
				lp.y = TerrainScale * TexturePatchSize * scale - lp.y;
				vertices[i].pos = tile_pos + (lp) * tile_scale + tangent*(road_size*tile_scale*0.5f);
				vertices[i].rhw = 1;
				if (p % 2 == 0)
				{
					vertices[i].u = 0;
					vertices[i].v = 0;
				}
				else
				{
					vertices[i].u = 0;
					vertices[i].v = v;// 1
				}
				vertices[i++].color = 0xFFFFFFFF;

				//Vec3 tangent = Vec3(1, 0, 0);
				vertices[i].pos = tile_pos + (lp) * tile_scale - tangent*(road_size*tile_scale*0.5f);
				vertices[i].rhw = 1;
				if (p % 2 == 0)
				{
					vertices[i].u = 1;
					vertices[i].v = 0;
				}
				else
				{
					vertices[i].u = 1;
					vertices[i].v = v;// 1;
				}
				vertices[i++].color = 0xFFFFFFFF;
				if (p + 1 < points.size())
					v += (points[p] - points[p + 1]).length()*0.05;
			}

			renderer->SetShader(resources.get_shader("Shaders/guitexture.txt"));

			CVertexBuffer b;

			VertexElement elm8[] = { { ELEMENT_FLOAT4, USAGE_POSITION },
			{ ELEMENT_COLOR, USAGE_COLOR },
			{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };
			b.SetVertexDeclaration(renderer->GetVertexDeclaration(elm8, 3));

			b.Data(vertices, sizeof(TLVERTEX2) * i, sizeof(TLVERTEX2));

			b.Bind();

			//Draw image
			renderer->DrawPrimitive(PT_TRIANGLEFAN, 0, i);
		}
	}

	renderer->SetPixelTexture(0, 0);


	//then mark it in the indirection texture
	{
		renderer->SetRenderTarget(0, this->indirection_texture);
		vp.Height = vp.Width = 2048 / TexturePatchSize;
		vp.X = vp.Y = 0;
		vp.MinZ = 0;
		vp.MaxZ = 1;
		renderer->SetViewport(&vp);

		Rect r;
		r.bottom = x;
		r.top = x + scale;
		r.right = y;
		r.left = y + scale;
		//use r and g for offset, a for scale
		int tileno = num;
		int offx = (tileno / tile_dim);
		int offy = (tileno % tile_dim);
		renderer->DrawRect(&r, COLOR_ARGB(255, scale, offx, offy));
	}

	//restore viewport
	renderer->SetViewport(&oldvp);
	renderer->SetRenderTarget(0, &ort);

	return num;
}

void HeightmapTerrainSystem::MarkTileFreed(int num)
{
	if (num >= 0)
		this->free_tiles.push_back(num);
}

struct HeightMapInfo
{
	int terrainWidth;
	int terrainHeight;
	float* heightMap;
};
bool HeightMapLoad(char* filename, HeightMapInfo &hminfo)
{
	FILE *filePtr;							// Point to the current position in the file
	BITMAPFILEHEADER bitmapFileHeader;		// Structure which stores information about file
	BITMAPINFOHEADER bitmapInfoHeader;		// Structure which stores information about image
	int imageSize, index;

	// Open the file
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return 0;

	// Read bitmaps header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	// Read the info header
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	// Get the width and height (width and length) of the image
	hminfo.terrainWidth = bitmapInfoHeader.biWidth;
	hminfo.terrainHeight = bitmapInfoHeader.biHeight;

	// Size of the image in bytes. the 3 represents RBG (byte, byte, byte) for each pixel
	imageSize = hminfo.terrainWidth * hminfo.terrainHeight * bitmapInfoHeader.biBitCount / 8;

	RGBQUAD* colors = 0;
	if (bitmapInfoHeader.biBitCount == 8) {
		colors = new RGBQUAD[256];
		fread(colors, sizeof(RGBQUAD), 256, filePtr);
	}
	//hminfo.terrainWidth = 2048;
	//hminfo.terrainHeight = 2048;
	//imageSize = 2048*2048;

	// Initialize the array which stores the image data
	unsigned char* bitmapImage = new unsigned char[imageSize];

	// Set the file pointer to the beginning of the image data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Store image data in bitmapImage
	fread(bitmapImage, 1, imageSize, filePtr);

	// Close file
	fclose(filePtr);

	// Initialize the heightMap array (stores the vertices of our terrain)
	hminfo.heightMap = new float[(hminfo.terrainWidth) * (hminfo.terrainHeight)];

	// We use a greyscale image, so all 3 rgb values are the same, but we only need one for the height
	// So we use this counter to skip the next two components in the image data (we read R, then skip BG)
	int k = 0;

	// We divide the height by this number to "water down" the terrains height, otherwise the terrain will
	// appear to be "spikey" and not so smooth.
	float heightFactor = 0.25f;// 1;//256.0f/(256.0f*256.0f);

	// Read the image data into our heightMap array
	for (int j = 0; j < hminfo.terrainHeight; j++)
	{
		for (int i = 0; i < hminfo.terrainWidth; i++)
		{
			unsigned char height = bitmapImage[k];
			//help y axis is flipped in texture
			//this fixes it here, but collisions are now broken
			index = ((hminfo.terrainHeight) * (2047 - j)) + i;

			//hminfo.heightMap[index].x = (float)i;
			hminfo.heightMap[index] = ((float)height)*heightFactor;
			//hminfo.heightMap[index].z = (float)j;

			k += 1;
		}
	}

	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

bool R16HeightMapLoad(char* filename, HeightMapInfo &hminfo)
{
	FILE *filePtr;							// Point to the current position in the file
	BITMAPFILEHEADER bitmapFileHeader;		// Structure which stores information about file
	BITMAPINFOHEADER bitmapInfoHeader;		// Structure which stores information about image
	int imageSize, index;

	// Open the file
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return 0;

	// Read bitmaps header
	//fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1,filePtr);

	// Read the info header
	//fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	// Get the width and height (width and length) of the image
	hminfo.terrainWidth = 2048;//bitmapInfoHeader.biWidth;
	hminfo.terrainHeight = 2048;//bitmapInfoHeader.biHeight;

	// Size of the image in bytes. the 3 represents RBG (byte, byte, byte) for each pixel
	imageSize = hminfo.terrainWidth * hminfo.terrainHeight * 2;//bitmapInfoHeader.biBitCount/8;

	/*RGBQUAD* colors = 0;
	if(bitmapInfoHeader.biBitCount == 8) {
	colors = new RGBQUAD[256];
	fread(colors,sizeof(RGBQUAD), 256, filePtr);
	}*/
	//hminfo.terrainWidth = 2048;
	//hminfo.terrainHeight = 2048;
	//imageSize = 2048*2048;

	// Initialize the array which stores the image data
	unsigned short* bitmapImage = new unsigned short[imageSize / 2];

	// Set the file pointer to the beginning of the image data
	//fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Store image data in bitmapImage
	fread(bitmapImage, 1, imageSize, filePtr);

	// Close file
	fclose(filePtr);

	// Initialize the heightMap array (stores the vertices of our terrain)
	hminfo.heightMap = new float[(hminfo.terrainWidth) * (hminfo.terrainHeight)];

	// We use a greyscale image, so all 3 rgb values are the same, but we only need one for the height
	// So we use this counter to skip the next two components in the image data (we read R, then skip BG)
	int k = 0;

	// We divide the height by this number to "water down" the terrains height, otherwise the terrain will
	// appear to be "spikey" and not so smooth.
	float heightFactor = 0.0075f; //0.1f;// 0.1f;// 0.00390625f / 4.0f;//256.0f/(256.0f*256.0f);
	//heightFactor = (1.0f)/(235.31f - (-43.17f));
	// Read the image data into our heightMap array
	for (int j = 0; j < hminfo.terrainHeight; j++)
	{
		for (int i = 0; i < hminfo.terrainWidth; i++)
		{
			unsigned short height = bitmapImage[k];
			//help y axis is flipped in texture
			//this fixes it here, but collisions are now broken
			index = ((hminfo.terrainHeight) * (/*2047-*/j)) + i;

			//hminfo.heightMap[index].x = (float)i;
			hminfo.heightMap[index] = ((float)height)*heightFactor;
			//hminfo.heightMap[index].z = (float)j;

			k += 1;
		}
	}

	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

void HeightmapTerrainSystem::SaveHeightmap(const char* file)
{
	//arbitrary scale will be....

	std::ofstream out(file, std::ios::binary);
	for (int i = 0; i < this->world_size; i++)
	{
		for (int y = 0; y < this->world_size; y++)
		{
			unsigned short value = this->heights[y*this->world_size + i] * (1.0f / 0.015f);
			out.write((char*)&value, 2);
		}
	}
}

void HeightmapTerrainSystem::GenerateHeightmap()
{
	//HeightMapInfo info;
	//R16HeightMapLoad("Content/heightmap.r16", info);

	//lets generate our own heights

	this->heights = new float[2048 * 2048];// info.heightMap;
	this->world_size = 2048;// info.terrainHeight;

	int ds_size = this->world_size*this->world_size / (4 * 4);
	this->heights_ds = new float[ds_size];

	//ok procedural time, generate map with mountains and such, maybe a river
	//	then add a few random objectives, spawn some enemies at each
	//	need to make world 2x as large and somehow hide edges....

	//generate some buildings
	//generate some heights
	const float hill_thresh = 0.4f;
	const float mountain_thresh = 0.75f;
	const float thresh = 0.3f;
	for (int x = 0; x < this->world_size; x++)
	{
		for (int y = 0; y < this->world_size; y++)
		{
			int index = ((this->world_size) * (x)) + y;

			float is_mountain = noise2d_perlin_abs(x / 260.0f, y / 260.0f, 41865, 2, 0.5f);

			//float scale = 5*(is_mountain - thresh)*(1.0f / (1.0f - thresh));
			float base_height = (noise2d_perlin_abs(x / 160.0f, y / 160.0f, 1586541, 6, 0.5));// +1.0f) * 20;// +100;

			if (is_mountain > hill_thresh)
			{
#undef min
				this->heights[index] = base_height*std::min(100.0f, 100 * (is_mountain - hill_thresh) / (mountain_thresh - hill_thresh)) + 100;
			}
			else
				this->heights[index] = 100;

			if (is_mountain < 0.05)
				this->heights[index] = 400;
			//if (is_mountain > )
			//float height = 0;
			//if (scale > 0)
			//	height = base_height*scale;
			//else
			//	height = base_height;
			//this->heights[index] = height+100;// noise2d_perlin(x / 80.0f, y / 80.0f, 1586541, 2, 0.25) * 50 + 100;
		}
	}
	//initialize all of this to 0
	/*for (int i = 0; i < ds_size; i++)
	this->heights_ds[i] = 0;

	//downsample for the heights
	for (int x = 0; x < this->world_size; x++)
	{
	for (int y = 0; y < this->world_size; y++)
	{
	int index = ((this->world_size) * (x)) + y;
	int ds_index = ((this->world_size/4) * (x/4)) + y/4;

	if (this->heights[index] > this->heights_ds[ds_index])
	this->heights_ds[ds_index] = this->heights[index];
	}
	}*/
	this->SetupChunks();
}

void HeightmapTerrainSystem::LoadHeightmap(const char* file)
{
	HeightMapInfo info;
	R16HeightMapLoad("Content/heightmap.r16", info);

	//lets generate our own heights

	this->heights = info.heightMap;
	this->world_size = info.terrainHeight;

	int ds_size = this->world_size*this->world_size / (4 * 4);
	this->heights_ds = new float[ds_size];

	//initialize all of this to 0
	/*for (int i = 0; i < ds_size; i++)
	this->heights_ds[i] = 0;

	//downsample for the heights
	for (int x = 0; x < this->world_size; x++)
	{
	for (int y = 0; y < this->world_size; y++)
	{
	int index = ((this->world_size) * (x)) + y;
	int ds_index = ((this->world_size/4) * (x/4)) + y/4;

	if (this->heights[index] > this->heights_ds[ds_index])
	this->heights_ds[ds_index] = this->heights[index];
	}
	}*/

	this->SetupChunks();
}

void HeightmapTerrainSystem::SetupChunks()
{
	//for number of players
	for (int i = 0; i < 4; i++)
	{
		grid[i] = new QuadTree*[(world_size / patch_size)*(world_size / patch_size)];
		for (int x = 0; x < world_size / patch_size; x++)
		{
			for (int y = 0; y < world_size / patch_size; y++)
			{
				grid[i][x*(world_size / patch_size) + y] = new QuadTree(x*patch_size*TerrainScale, y*patch_size*TerrainScale, 512, this);
			}
		}
	}
}

VertexDeclaration terrain_vd;
IMaterial* terrain_mat;
void HeightmapTerrainSystem::Load(float terrain_scale)
{
	bool loaded = this->grass ? true : false;

	if (loaded)
		return;
	//todo then use terrain system in an example project
	//this will help me work out any silly bugs with dependencies on random function calls/variables somewhere
	//create material
	TerrainScale = terrain_scale;
	TerrainMaterial* mt = new TerrainMaterial;
	this->material = mt;
	this->material->skinned = false;
	this->material->alpha = false;
	this->material->alphatest = false;
	this->material->shader_name = "Shaders/terrain_shadow.shdr";
	this->material->shader_builder = true;
	this->material->SetDefine("TERRAIN_SCALE", std::to_string(TerrainScale));
	this->material->SetDefine("TILE_SCALE", std::to_string(32 / TexturePatchSize));
	this->material->filter = FilterMode::Linear;
	this->material->cullmode = CULL_CW;
	this->my_material = mt;

	terrain_mat = this->material;

	VertexElement elm9[] = {
		{ ELEMENT_FLOAT3, USAGE_POSITION },
		{ ELEMENT_FLOAT2, USAGE_TEXCOORD }
	};
	this->vertex_declaration = renderer->GetVertexDeclaration(elm9, 2);
	terrain_vd = this->vertex_declaration;

	//load all textures
	grass = resources.get_unsafe<CTexture>("grass.jpg");
	snow = resources.get_unsafe<CTexture>("snow.jpg");
	rock = resources.get_unsafe<CTexture>("rock.png");
	road = resources.get_unsafe<CTexture>("road.jpg");
	noise = resources.get_unsafe<CTexture>("perturb.png");

	nmap = 0;

	if (loaded == false)
	{
		D3D11_SAMPLER_DESC comparisonSamplerDesc;
		ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
		comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.BorderColor[0] = 1.0f;
		comparisonSamplerDesc.BorderColor[1] = 1.0f;
		comparisonSamplerDesc.BorderColor[2] = 1.0f;
		comparisonSamplerDesc.BorderColor[3] = 1.0f;
		comparisonSamplerDesc.MinLOD = 0.f;
		comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		comparisonSamplerDesc.MipLODBias = 0.f;
		comparisonSamplerDesc.MaxAnisotropy = 0;
		comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;//LESS_EQUAL;
		comparisonSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//comparisonSamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;// D3D11_FILTER_MIN_MAG_MIP_POINT;
		//test this and try to get it to look better
		// Point filtered shadows can be faster, and may be a good choice when
		// rendering on hardware with lower feature levels. This sample has a
		// UI option to enable/disable filtering so you can see the difference
		// in quality and speed.

		auto res = renderer->device->CreateSamplerState(
			&comparisonSamplerDesc,
			&sampler
			);
		if (FAILED(res))
			throw 7;

		D3D11_SAMPLER_DESC comparisonSamplerDesc2;
		ZeroMemory(&comparisonSamplerDesc2, sizeof(D3D11_SAMPLER_DESC));
		comparisonSamplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		comparisonSamplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		comparisonSamplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc2.BorderColor[0] = 1.0f;
		comparisonSamplerDesc2.BorderColor[1] = 1.0f;
		comparisonSamplerDesc2.BorderColor[2] = 1.0f;
		comparisonSamplerDesc2.BorderColor[3] = 1.0f;
		comparisonSamplerDesc2.MinLOD = 0.f;
		comparisonSamplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;
		comparisonSamplerDesc2.MipLODBias = 0.f;
		comparisonSamplerDesc2.MaxAnisotropy = 0;
		comparisonSamplerDesc2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;//LESS_EQUAL;
		comparisonSamplerDesc2.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		// Point filtered shadows can be faster, and may be a good choice when
		// rendering on hardware with lower feature levels. This sample has a
		// UI option to enable/disable filtering so you can see the difference
		// in quality and speed.

		res = renderer->device->CreateSamplerState(
			&comparisonSamplerDesc2,
			&textureSampler
			);
		if (FAILED(res))
			throw 7;
	}

	//world_size = 1024 * 2;
	const int size = 2048;
	this->tile_texture = CRenderTexture::Create(TextureAtlasSize, TextureAtlasSize, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);// DXGI_FORMAT_R24G8_TYPELESS);
	//this gives 256 tiles, can reduce it later if necessary

	//color texture
	this->indirection_texture = CRenderTexture::Create(size / TexturePatchSize, size / TexturePatchSize, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);// DXGI_FORMAT_R24G8_TYPELESS);

	mt->grass = grass;
	mt->rock = rock;
	mt->sampler = this->sampler;
	mt->textureSampler = this->textureSampler;
	mt->indirection = this->indirection_texture;
	mt->tiles = this->tile_texture;
}

inline float linearInterpolation(float x0, float x1, float t)
{
	return x0 + (x1 - x0)*t;
};

inline float biLinearInterpolation(float x0y0, float x1y0, float x0y1, float x1y1, float x, float y){
	float tx = x;
	float ty = y;

	/*double tx = x;
	double ty = y;*/
	float u = linearInterpolation(x0y0, x1y0, tx);
	float v = linearInterpolation(x0y1, x1y1, tx);
	return linearInterpolation(u, v, ty);
};

float HeightmapTerrainSystem::GetHeight(float x, float y)
{
	x /= TerrainScale;
	y /= TerrainScale;
	if (x >= 0 && y >= 0 && x < 2047 && y < 2047)
	{
		int ix = floor(x);
		int iy = floor(y);
		float s00 = this->heights[iy * 2048 + ix];
		float s01 = this->heights[iy * 2048 + ix + 1];
		float s10 = this->heights[(iy + 1) * 2048 + ix];
		float s11 = this->heights[(iy + 1) * 2048 + ix + 1];
		return biLinearInterpolation(s00, s01, s10, s11, x - ix, y - iy);
		//return s00+(s01-s00)*(x-ix);//this->heights[y*2048+x];
	}

	return 0;
}

float HeightmapTerrainSystem::GetRoughHeight(float x, float y)
{
	x /= TerrainScale;// *4;
	y /= TerrainScale;// *4;
	if (x >= 0 && y >= 0 && x < 2047 && y < 2047)
	{
		int ix = floor(x);
		int iy = floor(y);
		float s00 = this->heights[iy * 2048 + ix];
		//float s01 = this->heights[iy * 2048 + ix + 1];
		//float s10 = this->heights[(iy + 1) * 2048 + ix];
		//float s11 = this->heights[(iy + 1) * 2048 + ix + 1];
		return s00;// biLinearInterpolation(s00, s01, s10, s11, x - ix, y - iy);
		//return s00+(s01-s00)*(x-ix);//this->heights[y*2048+x];
	}

	return 0;
}

float HeightmapTerrainSystem::GetHeightAndNormal(float x, float y, Vec3& normal)
{
	x /= TerrainScale;
	y /= TerrainScale;
	if (x >= 0 && y >= 0 && x < 2047 && y < 2047)
	{
		int ix = floor(x);
		int iy = floor(y);
		float s00 = this->heights[iy * 2048 + ix];
		float s01 = this->heights[iy * 2048 + ix + 1];
		float s10 = this->heights[(iy + 1) * 2048 + ix];
		float s11 = this->heights[(iy + 1) * 2048 + ix + 1];

		//float s01 = (float)(noise2d_perlin((v->x-1)/26, v->z/26, 123456789, 2, 0.4)*8.0);
		//float s21 = (float)(noise2d_perlin((v->x+1)/26, v->z/26, 123456789, 2, 0.4)*8.0);
		//float s10 = (float)(noise2d_perlin(v->x/26, (v->z-1)/26, 123456789, 2, 0.4)*8.0);
		//float s12 = (float)(noise2d_perlin(v->x/26, (v->z+1)/26, 123456789, 2, 0.4)*8.0);
		//Vec3 va = Vec3(2,0,s21-s01);
		//Vec3 vb = Vec3(0,2,s12-s10);
		//Vec3 n = va.cross(vb);

		//lets try using 3 positions to get the tangent vectors, then getting the normal from these
		Vec3 va = Vec3(TerrainScale, s01 - s00, 0);
		Vec3 vb = Vec3(0, s10 - s00, TerrainScale);
		normal = vb.cross(va).getnormal();
		float res = biLinearInterpolation(s00, s01, s10, s11, x - ix, y - iy);
		//renderer->DrawNormals(Vec3(x, res, y), va, normal, vb);
		return res;
	}
	normal = Vec3(0, 1, 0);
	return 0;
}

float HeightmapTerrainSystem::GetHeightAndVectors(float x, float y, Vec3& normal, Vec3& xtangent, Vec3& ytangent)
{
	x /= TerrainScale;
	y /= TerrainScale;
	if (x >= 0 && y >= 0 && x < 2047 && y < 2047)
	{
		int ix = floor(x);
		int iy = floor(y);
		float s00 = this->heights[iy * 2048 + ix];
		float s01 = this->heights[iy * 2048 + ix + 1];
		float s10 = this->heights[(iy + 1) * 2048 + ix];
		float s11 = this->heights[(iy + 1) * 2048 + ix + 1];

		//float s01 = (float)(noise2d_perlin((v->x-1)/26, v->z/26, 123456789, 2, 0.4)*8.0);
		//float s21 = (float)(noise2d_perlin((v->x+1)/26, v->z/26, 123456789, 2, 0.4)*8.0);
		//float s10 = (float)(noise2d_perlin(v->x/26, (v->z-1)/26, 123456789, 2, 0.4)*8.0);
		//float s12 = (float)(noise2d_perlin(v->x/26, (v->z+1)/26, 123456789, 2, 0.4)*8.0);
		//Vec3 va = Vec3(2,0,s21-s01);
		//Vec3 vb = Vec3(0,2,s12-s10);
		//Vec3 n = va.cross(vb);

		//lets try using 3 positions to get the tangent vectors, then getting the normal from these
		xtangent = Vec3(1, s01 - s00, 0).getnormal();
		ytangent = Vec3(0, s10 - s00, 1).getnormal();
		normal = ytangent.cross(xtangent).getnormal();
		float res = biLinearInterpolation(s00, s01, s10, s11, x - ix, y - iy);
		//renderer->DrawNormals(Vec3(x, res, y), va, normal, vb);
		return res;
	}
	normal = Vec3(0, 1, 0);
	return 0;
}

void HeightmapTerrainSystem::SetHeight(int x, int y, float z)
{
	x /= TerrainScale;
	y /= TerrainScale;
	if (x < 2048 & y < 2048 && y >= 0 && x >= 0)
		this->heights[y * 2048 + x] = z;
}

void HeightmapTerrainSystem::UpdateHeights()
{
	for (int i = 0; i < 4; i++)
	{
		//grid[i] = new QuadTree*[(world_size / patch_size)*(world_size / patch_size)];
		for (int x = 0; x < world_size / patch_size; x++)
		{
			for (int y = 0; y < world_size / patch_size; y++)
			{
				auto patch = grid[i][x*(world_size / patch_size) + y]->Root();
				patch->RegenVertices(this->heights);
			}
		}
	}
}

void HeightmapTerrainSystem::GenerateNormals()
{
	//first: make floating point texture with heights
	CTexture t;

	if (this->hmap.texture == 0)
	{
		auto tex = CTexture::Create(this->world_size, this->world_size, DXGI_FORMAT_R32_FLOAT, (const char*)this->heights);
		hmap.texture_rv = tex->texture_rv;
		hmap.texture = tex->texture;
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE sr;
		renderer->context->Map(this->hmap.texture, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &sr);
		memcpy(sr.pData, this->heights, 4 * this->world_size*this->world_size);
		renderer->context->Unmap(this->hmap.texture, 0);
	}

	//now we make a rendertarget

	//allocate shadow maps
	if (this->nmap == 0)
	{
		auto rt = CRenderTexture::Create(this->world_size, this->world_size, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);//DXGI_FORMAT_D24_UNORM_S8_UINT);
		this->nmap = rt;
	}

	render = [=]()
	{
		PROFILE("Generate Normals");
		auto myrt = static_cast<CRenderTexture*>(this->nmap);
		CRenderTexture rt = *myrt;
		rt.created = false;

		auto ort = renderer->GetRenderTarget(0);
		renderer->SetRenderTarget(0, &rt);

		renderer->SetPixelTexture(0, this->hmap);

		renderer->EnableAlphaBlending(false);

		//ok, lets adjust the viewport
		Viewport vp;
		Viewport oldvp;
		renderer->GetViewport(&oldvp);

		vp.Height = vp.Width = this->world_size;
		vp.X = vp.Y = 0;
		vp.MinZ = 0;
		vp.MaxZ = 1;
		renderer->SetViewport(&vp);


		//bind shaders, render full screen quad and bam
		renderer->SetShader(resources.get_unsafe<CShader>("Shaders/generate_normals.shdr"));

		struct data
		{
			Vec4 lightdir;
			float res;
		} td;
		td.lightdir = Vec4(r.GetSunLightDirection());
		td.res = this->world_size;
		renderer->shader->cbuffers["Data"].UploadAndSet(&td, sizeof(data));

		//draw full screen quad
		renderer->DrawFullScreenQuad();

		renderer->SetViewport(&oldvp);

		renderer->SetRenderTarget(0, &ort);
	};

	this->need_to_reload_normals = true;
	//delete myrt;

	//RELEASE IT ALL, besides the updated texture

	//then just need to save it to file
}

int HeightmapTerrainSystem::AddRoad(RoadPoint* points, unsigned int count)
{
	//calculate bounding box

	//lets just add to one quarter of map so far
	RoadData data;
	data.points = points;
	data.size = count;

	AABB bounds(100000, -100000, 100000, -100000, 100000, -100000);
	for (int i = 0; i < count; i++)
	{
		Vec3 half_normal = Vec3(0, 0, 1)*(points[i].width/2);
		//todo, need to include width
		bounds.FitPoint(points[i].pos + half_normal);
		bounds.FitPoint(points[i].pos - half_normal);
	}
	data.bounds = bounds;
	this->roads.push_back(data);

	//ok, we got it now lets add it to our nodes
	//lets have each node do its own bounds checks, just add it to everything

	//todo this view system needs to be redone, this is SUPER dumb, need to take the max lod of each viewer for all points to load,
	//but then only render as much as that viewer needs
	//store a lod per viewer in each patch
	for (int i = 0; i < 4; i++)
	{
		//grid[i] = new QuadTree*[(world_size / patch_size)*(world_size / patch_size)];
		for (int x = 0; x < world_size / patch_size; x++)
		{
			for (int y = 0; y < world_size / patch_size; y++)
			{
				auto patch = grid[i][x*(world_size / patch_size) + y]->Root();
				patch->AddRoad(this->roads.size() - 1, &data);
				//patch->RegenVertices(this->heights);
			}
		}
	}
	return this->roads.size() - 1;
}