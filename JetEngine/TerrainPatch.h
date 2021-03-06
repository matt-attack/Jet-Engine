#pragma once
#include "Graphics/CRenderer.h"
#include "Graphics/Renderer.h"
#include "Graphics/CVertexBuffer.h"
#include "Graphics/CIndexBuffer.h"

extern float TerrainScale;

enum TerrainDefs
{
	PatchSizePower = 5,//2^x = PatchSize
	PatchSize = 1 << PatchSizePower,// 32,//16,

	PatchMaxLOD = PatchSizePower * 2,//for 32, or 8 for 16

	//todo, lets use this granularity
	TexturePatchSize = 32,

	//this looks good enough, recommend changing to smaller tiles, but also smaller patches to even it out and use less texture memory
	TextureAtlasSize = 4096*2,
	TextureAtlasTileSize = 256*2,
};

struct TerrainVertex
{
	float x,y,z;
	float u, v;
};

class HeightmapTerrainSystem;
class TerrainPatch
{
	friend class HeightmapTerrainSystem;

	CVertexBuffer vbuffer;
	CIndexBuffer ibuffer;

	int size;
	int wx, wy;//world x and y positions
	
public:
	float min_height_, max_height_;//max and min heights

	int LoD;
	TerrainPatch(int wx, int wy, int size);
	~TerrainPatch(void);

	//worldspace coords put in
	void GenerateVertices(float* heights);

	//lod = triangulation level 0 = source, 1 = half, 2 = quarter, 3 = eighth
	//bools indicate if that neighbor is a higher LOD
	//if it is higher, need to make T joints
	//lets make a recursive patch generation algorithm
	//every time split along the longest edge, so alternate between midpoint and sides
	unsigned short* indices;
	int index;
	//gives the right angle x and right angle y
	enum Orientation
	{
		ORIENT_L, 
		//    /|
		//    \|

		ORIENT_R, 
		//   |\
		//   |/

		ORIENT_B, 
		//   --
		//   \/

		ORIENT_T, 
		//   /\
		//   --

		ORIENT_LB,
		/*   |\    */
		/*   |_\   */

		ORIENT_RB,
		/*    /|   */
		/*   /_|   */


		ORIENT_LT,
		/*    __   */
		/*   | /   */
		/*   |/    */

		ORIENT_RT 
		/*   __    */
		/*   \ |   */
		/*    \|   */
	};
	void BuildIndicesRecurse(int level, int lod, int rax, int ray, Orientation o);

	bool xi, xd, yi, yd;
	void GenerateIndices(int lod, bool xi, bool xd, bool yi, bool yd);

	void Render(CRenderer* r, CCamera* cam);
	void Render(CRenderer* r, const CCamera* cam, std::vector<RenderCommand>* queue, HeightmapTerrainSystem* root);
};

