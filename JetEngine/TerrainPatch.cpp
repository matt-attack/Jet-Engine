#include "TerrainPatch.h"
#include "Util\Noise.h"

#include <algorithm>

float TerrainScale = 1.0;

#undef min

//#define USE_SKIRTS

#ifdef USE_SKIRTS
#define Vertex(x,y) (x+1)*(PatchSize+3)+(y+1)
#else
#define Vertex(x,y) (x)*(PatchSize+1)+(y)
#endif

TerrainPatch::TerrainPatch(int wx, int wy, int size) : wx(wx), wy(wy), size(size)
{
	miny = -100000;
	maxy = 100000;
	LoD = -1;
}

TerrainPatch::~TerrainPatch(void)
{
}

void TerrainPatch::BuildIndicesRecurse(int level, int lod, int rax, int ray, Orientation o)
{
	int inc = 1 << (PatchSizePower - level);
	switch (o)
	{
	case ORIENT_L:
	{
		//    /|
		//   <-|
		//    \|

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(level, ++lod, rax + inc, ray, ORIENT_RB);
			BuildIndicesRecurse(level, lod, rax + inc, ray, ORIENT_RT);
		}
		else
		{
			//insert edge
#ifndef USE_SKIRTS
			if (rax + inc == PatchSize && this->xi)
			{
				indices[index++] = Vertex(rax + inc, ray);//RA
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax + inc, ray - inc);

				indices[index++] = Vertex(rax + inc, ray);//RA
				indices[index++] = Vertex(rax + inc, ray + inc);
				indices[index++] = Vertex(rax, ray);
			}
#else
			if (rax + inc == PatchSize)
			{
				indices[index++] = Vertex(PatchSize + 1, ray + inc);//RA
				indices[index++] = Vertex(PatchSize, ray + inc);
				indices[index++] = Vertex(PatchSize, ray - inc);

				indices[index++] = Vertex(PatchSize + 1, ray + inc);//RA
				indices[index++] = Vertex(PatchSize, ray - inc);
				indices[index++] = Vertex(PatchSize + 1, ray - inc);
			}
#endif
			///there is a problem with one of the two outside edge triangles
			//	x decreasing side
			//insert edge
			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax + inc, ray - inc);
			indices[index++] = Vertex(rax + inc, ray + inc);
		}
		break;
	}
	case ORIENT_R:
	{
		//. .|\
										//   | \
										//   |--
		//   | /
		//  .|/

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(level, ++lod, rax - inc, ray, ORIENT_LB);
			BuildIndicesRecurse(level, lod, rax - inc, ray, ORIENT_LT);
		}
		else
		{
			//insert edge
#ifndef USE_SKIRTS
			if (rax - inc == 0 && this->xd)
			{
				indices[index++] = Vertex(rax - inc, ray + inc);
				indices[index++] = Vertex(rax - inc, ray);//RA
				indices[index++] = Vertex(rax, ray);

				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax - inc, ray);//RA
				indices[index++] = Vertex(rax - inc, ray - inc);
			}
#else
			if (rax - inc == 0)
			{
				indices[index++] = Vertex(0, ray + inc);//RA
				indices[index++] = Vertex(-1, ray + inc);
				indices[index++] = Vertex(-1, ray - inc);

				indices[index++] = Vertex(0, ray + inc);//RA
				indices[index++] = Vertex(-1, ray - inc);
				indices[index++] = Vertex(0, ray - inc);
			}
#endif
			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax - inc, ray + inc);
			indices[index++] = Vertex(rax - inc, ray - inc);
		}
		break;
	}
	case ORIENT_LT:
	{
		/*    __   */
		/*   |\/   */
		/*   |/    */

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(++level, ++lod, rax + inc / 2, ray + inc / 2, ORIENT_R);
			BuildIndicesRecurse(level, lod, rax + inc / 2, ray + inc / 2, ORIENT_B);
		}
		else
		{
			//insert edge
			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax + inc, ray);
			indices[index++] = Vertex(rax, ray + inc);

#ifdef USE_SKIRTS
			//top triangle
			if (ray == 0)
			{
				indices[index++] = Vertex(rax, -1);
				indices[index++] = Vertex(rax + inc, -1);
				indices[index++] = Vertex(rax + inc, 0);

				indices[index++] = Vertex(rax, 0);
				indices[index++] = Vertex(rax, -1);
				indices[index++] = Vertex(rax + inc, 0);
			}

			//left triangle
			if (rax == 0)
			{
				indices[index++] = Vertex(-1, ray);
				indices[index++] = Vertex(0, ray);
				indices[index++] = Vertex(-1, ray + inc);

				indices[index++] = Vertex(0, ray);
				indices[index++] = Vertex(0, ray + inc);
				indices[index++] = Vertex(-1, ray + inc);
			}
#else
			// A
			//top triangle
			if (ray == 0 && this->yd)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax + inc / 2, ray);
				indices[index++] = Vertex(rax + inc / 2, ray + inc / 2);

				indices[index++] = Vertex(rax + inc, ray);
				indices[index++] = Vertex(rax + inc / 2, ray + inc / 2);
				indices[index++] = Vertex(rax + inc / 2, ray);
			}

			//left triangle
			if (rax == 0 && xd)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax + inc / 2, ray + inc / 2);
				indices[index++] = Vertex(rax, ray + inc / 2);

				indices[index++] = Vertex(rax + inc / 2, ray + inc / 2);
				indices[index++] = Vertex(rax, ray + inc);
				indices[index++] = Vertex(rax, ray + inc / 2);
			}
#endif
		}
		break;
	}
	case ORIENT_LB:
	{
		/*   |\    */
		/*   |/\   */
		/*   ---   */

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(++level, ++lod, rax + inc / 2, ray - inc / 2, ORIENT_R);
			BuildIndicesRecurse(level, lod, rax + inc / 2, ray - inc / 2, ORIENT_T);
		}
		else
		{
			//insert edge
			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax, ray - inc);
			indices[index++] = Vertex(rax + inc, ray);
			//its this one
#ifdef USE_SKIRTS
			//bottom triangle
			if (ray == PatchSize)
			{
				indices[index++] = Vertex(rax, PatchSize);
				indices[index++] = Vertex(rax + inc, PatchSize);
				indices[index++] = Vertex(rax + inc, PatchSize + 1);

				indices[index++] = Vertex(rax, PatchSize);
				indices[index++] = Vertex(rax + inc, PatchSize + 1);
				indices[index++] = Vertex(rax, PatchSize + 1);

			}

			//left triangle
			if (rax == 0)
			{
				indices[index++] = Vertex(0, ray);
				indices[index++] = Vertex(-1, ray);
				indices[index++] = Vertex(-1, ray - inc);

				indices[index++] = Vertex(0, ray - inc);
				indices[index++] = Vertex(0, ray);
				indices[index++] = Vertex(-1, ray - inc);
			}
#else
			// Add two flange triangles on left border
			if (rax == 0 && this->xd)
			{
				indices[index++] = Vertex(rax, ray - inc / 2);
				indices[index++] = Vertex(rax, ray - inc);
				indices[index++] = Vertex(rax + inc / 2, ray - inc / 2);

				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax, ray - inc / 2);
				indices[index++] = Vertex(rax + inc / 2, ray - inc / 2);
			}
			// Add two flange triangles bottom border
			if (ray == PatchSize && this->yi)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax + inc / 2, ray - inc / 2);
				indices[index++] = Vertex(rax + inc / 2, ray);

				indices[index++] = Vertex(rax + inc / 2, ray);
				indices[index++] = Vertex(rax + inc / 2, ray - inc / 2);
				indices[index++] = Vertex(rax + inc, ray);
			}
#endif
		}
		break;
	}
	case ORIENT_T:
	{
		//   /|\
																																											//   ---
		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(level, ++lod, rax, ray + inc, ORIENT_RB);
			BuildIndicesRecurse(level, lod, rax, ray + inc, ORIENT_LB);
		}
		else
		{
#ifndef USE_SKIRTS
			//insert edge
			if (ray + inc == PatchSize && this->yi)
			{
				indices[index++] = Vertex(rax, ray + inc);//RA
				indices[index++] = Vertex(rax - inc, ray + inc);
				indices[index++] = Vertex(rax, ray);

				indices[index++] = Vertex(rax, ray + inc);//RA
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax + inc, ray + inc);
			}
#else
			if (ray + inc == PatchSize)
			{
				indices[index++] = Vertex(rax + inc, 0);//RA
				indices[index++] = Vertex(rax - inc, -1);
				indices[index++] = Vertex(rax + inc, -1);

				indices[index++] = Vertex(rax + inc, 0);//RA
				indices[index++] = Vertex(rax - inc, 0);
				indices[index++] = Vertex(rax - inc, -1);
			}
#endif

			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax + inc, ray + inc);
			indices[index++] = Vertex(rax - inc, ray + inc);
		}

		break;
	}
	case ORIENT_B:
	{
		//   ---
		//   \|/
		/*bool either = false;
		if (lod < this->LoD - ((ray - inc == 0 && yd) ? 1 : 0))//we need more lod)
		{
		either = true;
		BuildIndicesRecurse(level, lod+1, rax, ray - inc, ORIENT_RT);
		}
		if (lod < this->LoD - ((ray - inc == 0 && yd) ? 1 : 0))//lod < this->LoD)
		{
		either = true;
		BuildIndicesRecurse(level, lod+1, rax, ray - inc, ORIENT_LT);
		}*/

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(level, ++lod, rax, ray - inc, ORIENT_RT);
			BuildIndicesRecurse(level, lod, rax, ray - inc, ORIENT_LT);
		}

		//if (either == false)//else
		else
		{
#ifndef USE_SKIRTS
			//insert edge
			if (ray - inc == 0 && yd)
			{
				indices[index++] = Vertex(rax, ray - inc);//RA
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax - inc, ray - inc);

				indices[index++] = Vertex(rax, ray - inc);//RA
				indices[index++] = Vertex(rax + inc, ray - inc);
				indices[index++] = Vertex(rax, ray);
			}
#else
			if (ray - inc == 0)
			{
				indices[index++] = Vertex(rax + inc, PatchSize + 1);//RA
				indices[index++] = Vertex(rax - inc, PatchSize);
				indices[index++] = Vertex(rax + inc, PatchSize);

				indices[index++] = Vertex(rax + inc, PatchSize + 1);//RA
				indices[index++] = Vertex(rax - inc, PatchSize + 1);
				indices[index++] = Vertex(rax - inc, PatchSize);
			}
#endif

			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax - inc, ray - inc);
			indices[index++] = Vertex(rax + inc, ray - inc);
		}
		break;
	}
	case ORIENT_RB:
	{
		/*    /|   */
		/*   /\|   */
		/*   ---   */

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(++level, ++lod, rax - inc / 2, ray - inc / 2, ORIENT_L);
			BuildIndicesRecurse(level, lod, rax - inc / 2, ray - inc / 2, ORIENT_T);
		}
		else
		{
			//insert edge
			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax - inc, ray);
			indices[index++] = Vertex(rax, ray - inc);
#ifdef USE_SKIRTS
			//bottom triangle
			if (ray == PatchSize)
			{
				indices[index++] = Vertex(rax, PatchSize);
				indices[index++] = Vertex(rax - inc, PatchSize + 1);
				indices[index++] = Vertex(rax - inc, PatchSize);

				indices[index++] = Vertex(rax, PatchSize);
				indices[index++] = Vertex(rax, PatchSize + 1);
				indices[index++] = Vertex(rax - inc, PatchSize + 1);
			}

			//right triangle
			if (rax == PatchSize)
			{
				indices[index++] = Vertex(PatchSize + 1, ray);
				indices[index++] = Vertex(PatchSize, ray);
				indices[index++] = Vertex(PatchSize, ray - inc);

				indices[index++] = Vertex(PatchSize, ray - inc);
				indices[index++] = Vertex(PatchSize + 1, ray - inc);
				indices[index++] = Vertex(PatchSize + 1, ray);
			}
#else
			//bottom triangle
			if (ray == PatchSize && this->yi)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax - inc / 2, ray);
				indices[index++] = Vertex(rax - inc / 2, ray - inc / 2);

				indices[index++] = Vertex(rax - inc, ray);
				indices[index++] = Vertex(rax - inc / 2, ray - inc / 2);
				indices[index++] = Vertex(rax - inc / 2, ray);
			}

			//right triangle
			if (rax == PatchSize && this->xi)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax - inc / 2, ray - inc / 2);
				indices[index++] = Vertex(rax, ray - inc / 2);

				indices[index++] = Vertex(rax - inc / 2, ray - inc / 2);
				indices[index++] = Vertex(rax, ray - inc);
				indices[index++] = Vertex(rax, ray - inc / 2);
			}
#endif
		}
		break;
	}
	case ORIENT_RT:
	{
		/*   __    */
		/*   \/|   */
		/*    \|   */

		if (lod < this->LoD)//we need more lod)
		{
			BuildIndicesRecurse(++level, ++lod, rax - inc / 2, ray + inc / 2, ORIENT_B);
			BuildIndicesRecurse(level, lod, rax - inc / 2, ray + inc / 2, ORIENT_L);
		}
		else
		{
			//insert edge
			indices[index++] = Vertex(rax, ray);
			indices[index++] = Vertex(rax, ray + inc);
			indices[index++] = Vertex(rax - inc, ray);

#ifdef USE_SKIRTS
			//top triangle
			if (ray == 0)
			{
				indices[index++] = Vertex(rax, -1);
				indices[index++] = Vertex(rax - inc, 0);
				indices[index++] = Vertex(rax - inc, -1);

				indices[index++] = Vertex(rax, -1);
				indices[index++] = Vertex(rax, 0);
				indices[index++] = Vertex(rax - inc, 0);
			}

			//right triangle
			if (rax == PatchSize)
			{
				indices[index++] = Vertex(PatchSize + 1, ray);
				indices[index++] = Vertex(PatchSize, ray + inc);
				indices[index++] = Vertex(PatchSize, ray);

				indices[index++] = Vertex(PatchSize + 1, ray + inc);
				indices[index++] = Vertex(PatchSize, ray + inc);
				indices[index++] = Vertex(PatchSize + 1, ray);
			}
#else
			//top triangle
			if (ray == 0 && this->yd)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax - inc / 2, ray + inc / 2);
				indices[index++] = Vertex(rax - inc / 2, ray);

				indices[index++] = Vertex(rax - inc, ray);
				indices[index++] = Vertex(rax - inc / 2, ray);
				indices[index++] = Vertex(rax - inc / 2, ray + inc / 2);
			}

			//right triangle
			if (rax == PatchSize && this->xi)
			{
				indices[index++] = Vertex(rax, ray);
				indices[index++] = Vertex(rax, ray + inc / 2);
				indices[index++] = Vertex(rax - inc / 2, ray + inc / 2);

				indices[index++] = Vertex(rax - inc / 2, ray + inc / 2);
				indices[index++] = Vertex(rax, ray + inc / 2);
				indices[index++] = Vertex(rax, ray + inc);
			}
#endif
		}
		break;
	}
	}
}

void TerrainPatch::GenerateIndices(int lod, bool xi, bool xd, bool yi, bool yd)
{
	this->xi = xi;
	this->xd = xd;
	this->yi = yi;
	this->yd = yd;

	if (lod > PatchMaxLOD)
		lod = PatchMaxLOD;
	//todo: cache these
	//each neighbor with higher lod doubles the number of triangles on one side edge
	//only if my lod >= 1
	//so indices += numberoftriangles on side*3 = (

	if (this->LoD == lod)
		return;

	this->LoD = lod;

	//fix this
	int rounded = lod - lod % 2;
	int incr = PatchSize;
	if (rounded >= 2)
		incr = PatchSize / (rounded * 2);//to be safe

	incr /= 2;

	if (incr == 0)
		incr = 1;

	// int incr = lod >= 8 ? 1 : lod >= 6 ? 2 : lod >= 4 ? 4 : lod >= 2 ? 8 : 16;

	//with n*n vertices, there are (n-1)*(n-1) quads, so 2x that many triangles
	//number of vertices is PatchSize/inc
	//number of rects is 8 verts 7 rects, 4 verts 3 rects, 2 verts 1 rect = verts - 1
	int size = (1 << lod) * 6;//lod == 0 ? 6 : lod == 1 ? 12 : lod == 2 ? 24 : lod == 3 ? 48 : lod == 4 ? 96: lod == 5 ? 96*2 : 96*2*2;//size;

	//if (lod > 0)
	{
		if (xi)
			size += (PatchSize / incr) * 6;
		if (xd)
			size += (PatchSize / incr) * 6;
		if (yi)
			size += (PatchSize / incr) * 6;
		if (yd)
			size += (PatchSize / incr) * 6;
	}
	//for each higher level neighbor, that adds 
	//ex 4x4 vertex, needs to match with 8x8 one on side
	//thats 4 more vertices on the edge + 
	/*unsigned short**/ indices = new unsigned short[size];
	//implement bool stuff later
	/*	int*/ index = 0;
	this->BuildIndicesRecurse(0, 0, 0, 0, ORIENT_LT);
	this->BuildIndicesRecurse(0, 0, PatchSize, PatchSize, ORIENT_RB);
	//ok we need to calculate error metrics to determine the right LOD
	//dummy version!
	/*for (int x = 0; x < PatchSize; x+=incr)
	{
	for (int y = 0; y < PatchSize; y+=incr)
	{
	//lets do cw winding
	//top left triangle
	indices[index++] = Vertex(x,y);
	indices[index++] = Vertex(x+incr,y);
	indices[index++] = Vertex(x,y+incr);

	//bottom right triangle
	indices[index++] = Vertex(x+incr,y);
	indices[index++] = Vertex(x+incr,y+incr);
	indices[index++] = Vertex(x,y+incr);
	}
	}*/
#ifndef USE_SKIRTS
	//use the bools for lod thing
	int PATCH_SIZE = PatchSize;
	TerrainPatch* Other;
	/*int i = 0;
	//if (i >= m_HeightPatches)
	{
		int OtherLoD = PatchMaxLOD - 2//other lod;
		//round and divide by two
		OtherLoD = OtherLoD - OtherLoD % 2;
		//OtherLoD /= 2;

		//limits the LOD to 2
		//OtherLoD = 2;//works
		//OtherLoD = 4;//works
		//Other = &m_Patches[i - m_HeightPatches];
		if (OtherLoD > 0)//true)//Other->LoD > this->LoD)
		{
			//OtherLoD += 1;
			//the patch to the left is at a lower LoD
			//we need to reduce our left detail level
			for (int n = 0; n < index; ++n)
			{
				if ((indices[n] < (PATCH_SIZE + 1)) && (indices[n] % OtherLoD) != 0)
				{
					indices[n] -= indices[n] % OtherLoD;
				}
			}
		}
	}*/

	/*if (i <= m_NumPatches - m_HeightPatches)
	{
	Other = &m_Patches[i + m_HeightPatches];
	if (Other->LoD > this->LoD)
	{
	//Patch to the right is at a lower LoD
	//reduce right edge detail
	for (int n = 0; n < index; ++n)
	{
	if ((indices[n] >((PATCH_SIZE + 1)*PATCH_SIZE)) && (indices[n] % Other->LoD) != 0)
	{

	indices[n] -= indices[n] % Other->LoD;
	}
	}
	}
	}

	if (i % m_HeightPatches)
	{
	Other = &m_Patches[i - 1];

	if (Other->LoD > this->LoD)
	{
	//the patch to the top is at a lower LoD
	//we need to reduce our top detail level
	for (int n = 0; n < index; ++n)
	{
	if ((indices[n] % (PATCH_SIZE + 1) == 0) && (indices[n] % (Other->LoD * (PATCH_SIZE + 1))) != 0)
	{
	indices[n] -= (PATCH_SIZE + 1) * indices[n] % (Other->LoD * (PATCH_SIZE + 1));
	}
	}
	}
	}

	if ((i + 1) % m_HeightPatches)
	{
	Other = &m_Patches[i + 1];

	if (Other->LoD > this->LoD)
	{
	//the patch to the bottom is at a lower LoD

	//we need to reduce our bottom detail level

	for (int n = 0; n < index; ++n)
	{
	if (((indices[n] + 1) % (PATCH_SIZE + 1) == 0) && (indices[n] % (Other->LoD * (PATCH_SIZE + 1))) != 0)
	{
	indices[n] -= (PATCH_SIZE + 1) * indices[n] % (Other->LoD * (PATCH_SIZE + 1));
	}
	}
	}
	}*/
#endif
	ibuffer.Data(indices, index/*size*/ * 2, 0);
	delete[] indices;
}

void TerrainPatch::Render(CRenderer* r, CCamera* cam)
{
	//ok, add shader selection here
	ibuffer.Bind();
	vbuffer.Bind();

	renderer->stats.triangles += ibuffer.size / 6;
	renderer->stats.vertices += ibuffer.size / 2;

	r->context->DrawIndexed(ibuffer.size / 2, 0, 0);
}

#include "TerrainSystem.h"
extern IMaterial* terrain_mat;
void TerrainPatch::Render(CRenderer* r, CCamera* cam, std::vector<RenderCommand>* queue, HeightmapTerrainSystem* root)
{
	RenderCommand rc;
	rc.mesh.ib = &this->ibuffer;
	rc.mesh.vb = &this->vbuffer;
	rc.mesh.OutFrames = 0;
	rc.mesh.num_indices = ibuffer.size / 2;
	rc.mesh.primitives = ibuffer.size / 6;
	rc.material = terrain_mat;
	rc.alpha = false;
	rc.source = static_cast<Renderable*>(root);
	rc.material_instance.extra = 0;
	rc.position = Vec3(this->wx + PatchSize / 2, (this->maxy + this->miny) / 2, this->wy + PatchSize / 2);
	rc.radius = max((this->maxy - this->miny)/2, PatchSize);
	
	queue->push_back(rc);
}

extern VertexDeclaration terrain_vd;
int m_terrainWidth = (float)(2048.0f * TerrainScale);
int TEXTURE_REPEAT = (int)(50.0f*TerrainScale);
void TerrainPatch::GenerateVertices(float* heights)
{
#ifndef USE_SKIRTS
	const int d_size = (PatchSize + 1)*(PatchSize + 1);
#else
	const int d_size = (PatchSize + 3)*(PatchSize + 3);
#endif
	//ok, lets pad with 1 vertex on each side
	//	then we can do the flanges on the edge to fix any gaps
	TerrainVertex* data = new TerrainVertex[d_size];

	//calculate how much to increment the texture coordinates by.
	float incrementValue = ((float)TEXTURE_REPEAT / (float)m_terrainWidth) / (float)(size / PatchSize);

	//calculate how many times to repeat the texture.
	int incrementCount = m_terrainWidth / TEXTURE_REPEAT;

	//initialize the tu and tv coordinate values.
	float tuCoordinate = 0.0f;
	float tvCoordinate = 1.0f;

	//initialize the tu and tv coordinate indexes.
	int tuCount = 0;
	int tvCount = 0;

	float max = -100000;
	float min = 100000;
	for (int x = 0; x <= PatchSize; x++)
	{
		for (int y = 0; y <= PatchSize; y++)
		{
			auto v = &data[Vertex(x, y)];
			v->x = x*(size / PatchSize)*TerrainScale + wx;
			v->z = y*(size / PatchSize)*TerrainScale + wy;
			if (v->z == 2048 * TerrainScale || v->x == 2048 * TerrainScale)
				v->y = 10;
			else
				v->y = heights[((int)(v->z / TerrainScale)) * 2048 + ((int)(v->x / TerrainScale))];//(float)(noise2d_perlin(v->x/26, v->z/26, 123456789, 2, 0.4)*8.0);//y + wy;//height I guess

			v->u = tuCoordinate;
			v->v = tvCoordinate;
			if (v->y < min)
				min = v->y;
			if (v->y > max)
				max = v->y;

			//increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			//check if at the far right end of the texture and if so then start at the beginning again.
			if (tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}
		//increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		//check if at the top of the texture and if so then start at the bottom again.
		if (tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}
	this->maxy = max;
	this->miny = min;

#ifdef USE_SKIRTS
	//do the 4 corners, because they are wierd
	data[Vertex(-1, -1)].x = wx;
	data[Vertex(-1, -1)].y = 0;
	data[Vertex(-1, -1)].z = wy;
	data[Vertex(-1, PatchSize + 1)].x = wx;
	data[Vertex(-1, PatchSize + 1)].y = 0;
	data[Vertex(-1, PatchSize + 1)].z = wy + size;

	data[Vertex(PatchSize + 1, -1)].x = wx + size;
	data[Vertex(PatchSize + 1, -1)].y = 0;
	data[Vertex(PatchSize + 1, -1)].z = wy;
	data[Vertex(PatchSize + 1, PatchSize + 1)].x = wx + size;
	data[Vertex(PatchSize + 1, PatchSize + 1)].y = 0;
	data[Vertex(PatchSize + 1, PatchSize + 1)].z = wy + size;
	//add padded data for 4 edges
	//sample at the midpoints for these
	//top
	int inc = size / PatchSize;
	//top
	for (int x = 0; x <= PatchSize; x++)
	{
		auto v = &data[Vertex(x, -1)];
		v->x = x*inc + wx;
		v->y = 0;
		v->z = 0 * inc + wy;

		if (v->z > inc / 2 && v->z < 2048 - inc / 2 && v->x > inc / 2 && v->x < 2048 - inc / 2)
			v->y = std::min(heights[((int)v->z) * 2048 + ((int)v->x)], heights[((int)v->z) * 2048 + ((int)v->x + inc / 2)]) - 4;// -1; //heights[((int)v->z) * 2048 + ((int)v->x)];
		else
			v->y = 0;
	}
	//bottom
	for (int x = 0; x <= PatchSize; x++)
	{
		auto v = &data[Vertex(x, PatchSize + 1)];
		v->x = x*inc + wx;
		v->y = 0;
		v->z = 0 * inc + wy + size;
		if (v->z > inc / 2 && v->z < 2048 - inc / 2 && v->x > inc / 2 && v->x < 2048 - inc / 2)
			v->y = std::min(heights[((int)v->z) * 2048 + ((int)v->x)], heights[((int)v->z) * 2048 + ((int)v->x + inc / 2)]) - 4;// -1; //heights[((int)v->z) * 2048 + ((int)v->x)];
		else
			v->y = 0;
	}
	//left
	for (int y = 0; y <= PatchSize; y++)
	{
		auto v = &data[Vertex(-1, y)];
		v->x = 0 * inc + wx;
		v->y = 0;
		v->z = y*inc + wy;
		if (v->z > inc / 2 && v->z < 2048 - inc / 2 && v->x > inc / 2 && v->x < 2048 - inc / 2)
			v->y = std::min(heights[((int)v->z) * 2048 + ((int)v->x)], heights[((int)v->z + inc / 2) * 2048 + ((int)v->x)]) - 4;// -1; //heights[((int)v->z) * 2048 + ((int)v->x)];
		else
			v->y = 0;
	}
	//right
	for (int y = 0; y <= PatchSize; y++)
	{
		auto v = &data[Vertex(PatchSize + 1, y)];
		v->x = 0 * inc + wx + size;
		v->y = 0;
		v->z = y*inc + wy;
		if (v->z > 0 && v->z < 2048 - inc / 2 && v->x > 0 && v->x < 2048 - inc / 2)
			v->y = std::min(heights[((int)v->z) * 2048 + ((int)v->x)], heights[((int)v->z + inc / 2) * 2048 + ((int)v->x)]) - 4;// -1;
		else
			v->y = 0;
	}
#endif
	vbuffer.SetVertexDeclaration(terrain_vd);//renderer->GetVertexDeclaration(10));
	vbuffer.Data(data, d_size*sizeof(TerrainVertex), sizeof(TerrainVertex));

	delete[] data;
}
