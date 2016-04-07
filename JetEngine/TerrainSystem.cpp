#include "TerrainSystem.h"
#include "Util/Profile.h"
#include "camera.h"

#include "Graphics/CRenderer.h"
#include "Graphics/Renderable.h"
#include "Graphics/Renderer.h"
#include "Graphics/CTexture.h"
#include "Graphics/Shader.h"
#include "Graphics/RenderTexture.h"

#include <fstream>
#include <ostream>

#include "IMaterial.h"
class TerrainMaterial : public IMaterial
{
	
public:
	ID3D11SamplerState* sampler, *textureSampler;
	CTexture *nmap, *grass, *rock;
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
		renderer->context->PSSetShaderResources(0, 1, &nmap->texture);
		renderer->context->PSSetShaderResources(6, 1, &grass->texture);
		renderer->context->PSSetShaderResources(7, 1, &rock->texture);

		renderer->SetPrimitiveType(PT_TRIANGLELIST);
		
		renderer->EnableAlphaBlending(false);
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
public:
	int x, y;
	int size;

	AABB aabb;

	QuadTreeNode* northwest;
	QuadTreeNode* northeast;
	QuadTreeNode* southwest;
	QuadTreeNode* southeast;

	QuadTreeNode(QuadTreeNode* parent, int x, int y, int size)
	{
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
	}

	void Subdivide()
	{
		northwest = new QuadTreeNode(this, x, y, size / 2);
		northeast = new QuadTreeNode(this, x + size*TerrainScale / 2, y, size / 2);
		southwest = new QuadTreeNode(this, x, y + size*TerrainScale / 2, size / 2);
		southeast = new QuadTreeNode(this, x + size*TerrainScale / 2, y + size*TerrainScale / 2, size / 2);
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

		return lod;
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

				//this is bad, but oh well
				int lod = this->GetLOD(cam);

				this->patch->GenerateIndices(lod, true, true, true, true);
			}
			//if (this->patch->level == 8)
			//return;
			//render myself
			this->patch->Render(renderer, 0, queue);
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
	QuadTree(int x, int y, int size)
	{
		root = new QuadTreeNode(0, x, y, size);
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
	
	grass = 0;
	hmapt = 0;
	hmapv = 0;
	need_to_reload_normals = false;
	patch_size = 512;
	world_size = 1024 * 2;

	//for number of players
	for (int i = 0; i < 2; i++)
	{
		grid[i] = new QuadTree*[(world_size / patch_size)*(world_size / patch_size)];
		for (int x = 0; x < world_size / patch_size; x++)
		{
			for (int y = 0; y < world_size / patch_size; y++)
			{
				grid[i][x*(world_size / patch_size) + y] = new QuadTree(x*patch_size*TerrainScale, y*patch_size*TerrainScale, 512);
			}
		}
	}

	this->heights = 0;
}

HeightmapTerrainSystem::~HeightmapTerrainSystem()
{
	delete[] this->heights;
	this->sampler->Release();
	this->textureSampler->Release();
	this->hmapv->Release();
	this->hmapt->Release();

	for (int i = 0; i < 2; i++)
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
	GPUPROFILE("Terrain Render");

	this->temp_player = player;

	//generate normals if not done
	if (!done)
	{
		done = true;
		this->GenerateNormals();
	}

	if (this->need_to_reload_normals)
	{
		this->need_to_reload_normals = false;
		render();
	}

	this->my_material->nmap = this->nmap;


	r.AddRenderable(this);

	flipper++;// = 0;

	auto grid = this->grid[player];
	for (int x = 0; x < world_size / patch_size; x++)
	{
		for (int y = 0; y < world_size / patch_size; y++)
		{
			if (flipper++ % 2)//only updates half each frame
				grid[x*(world_size / patch_size) + y]->Root()->UpdateLOD(cam);
			//grid[x*(world_size / patch_size) + y]->Root()->Render(this->heights, cam);
		}
	}

	//renderer->EnableAlphaBlending(true);
	//renderer->SetTexture(0, rtview);
	//Rect r(0, 600, 400, 1000);
	//renderer->DrawRect(&r, 0xFFFFFFF);
	//renderer->EnableAlphaBlending(false);
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

VertexDeclaration terrain_vd;
IMaterial* terrain_mat;
void HeightmapTerrainSystem::Load()
{
	//create material
	TerrainMaterial* mt = new TerrainMaterial;
	this->material = mt;// IMaterial("terrain");
	this->material->skinned = false;
	this->material->alpha = false;
	this->material->alphatest = false;
	this->material->shader_name = "Shaders/terrain_shadow.shdr";
	this->material->shader_builder = true;
	//this->material->shader_ptr = shader_s;
	this->material->cullmode = CULL_CW;
	this->my_material = mt;

	terrain_mat = this->material;
	bool loaded = this->grass ? true : false;

	VertexElement elm9[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };
	this->vertex_declaration = renderer->GetVertexDeclaration(elm9, 2);
	terrain_vd = this->vertex_declaration;

	grass = resources.get<CTexture>("snow.jpg");
	rock = resources.get<CTexture>("rock.png");
	//implement teh texturing
	if (loaded == false)
	{
		HeightMapInfo info;
		R16HeightMapLoad("Content/heightmap.r16", info);

		this->heights = info.heightMap;
		this->world_size = info.terrainHeight;
	}

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
		comparisonSamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;// D3D11_FILTER_MIN_MAG_MIP_POINT;

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

	mt->grass = grass;
	mt->rock = rock;
	mt->sampler = this->sampler;
	mt->textureSampler = this->textureSampler;
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
	for (int i = 0; i < 2; i++)
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

	if (this->hmapv == 0)
	{
		auto tex = CTexture::Create(this->world_size, this->world_size, DXGI_FORMAT_R32_FLOAT, (const char*)this->heights);
		hmapv = tex->texture;
		hmapt = tex->data;
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE sr;
		renderer->context->Map(this->hmapt, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &sr);
		memcpy(sr.pData, this->heights, 4 * this->world_size*this->world_size);
		renderer->context->Unmap(this->hmapt, 0);
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

		renderer->SetPixelTexture(0, this->hmapv);

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
		renderer->SetShader(resources.get<CShader>("Shaders/generate_normals.shdr"));

		struct data
		{
			Vec4 lightdir;
			float res;
		} td;
		td.lightdir = Vec4(r.GetLightDirection());
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