#ifndef _BLOCKMODEL_HEADER
#define _BLOCKMODEL_HEADER

#include "../../Math/AABB.h"

#include "../Renderable.h"
#include "../CVertexBuffer.h"

typedef unsigned int uint;

enum BlockFaceDirection
{
	XIncreasing = (byte)1,
	XDecreasing = (byte)2,
	YIncreasing = (byte)4,
	YDecreasing = (byte)8,
	ZIncreasing = (byte)16,
	ZDecreasing = (byte)32,
	MAXIMUM
};

struct VERTEX2//pos, normal, texcoord
{
	float x, y, z;
	unsigned int color;
	float tu, tv;
};
#ifndef MATT_SERVER
class CVertexBuffer;
#endif

class BlockModel: public Renderable
{
public:
#ifndef MATT_SERVER
	CVertexBuffer v_buffer;// the pointer to the vertex buffer
	CTexture* tex;
#endif
	//Matrix4 _matrix;
	bool dirty;

	int Blocks[32][32][32];

	int _solidFaceCount;
	int _waterFaceCount;
	int _solidIndex;
	int _waterIndex;

	virtual void Load(bool fast = false);

#ifndef MATT_SERVER
	virtual void Render(const CCamera* cam, std::vector<RenderCommand>* queue);
	//virtual void Render(CRenderer* render);
#endif
	void Release();

	void SetPosition(int x, int y, int z);

	void AddBlock(int x, int y, int z, int blockType);
	void AddRectangle(int x, int y, int z, int dx, int dy, int dz, int color)//starts at pos and goes right, forward, and up
	{
		for (int xp = 0; xp < dx; xp++)
		{
			for (int yp = 0; yp < dy; yp++)
			{
				for (int zp = 0; zp < dz; zp++)
				{
					this->Blocks[x+xp][y+yp][z+zp] = color;
				}
			}
		}
	};

	void RemoveBlock(int x, int y, int z);

	int GetFaceTexture(int blocktype, BlockFaceDirection faceDir);
	void BuildBlockVertices(VERTEX2* vertexArray, int blockType, byte faceInfo, int x, int y, int z);

	void BuildFaceVertices(int x, int y, int z, VERTEX2* vertexArray, int faceDir, int blockType);

	BlockModel();
	~BlockModel();
};


struct VERTEX//pos, normal, texcoord
{
	float x, y, z;
	Vec3 n;
	float tu, tv;
	float a;
	float l;
};

struct BLOCKVERTEX {//12+4+8+4+4 bytes = 16 + 12 +4 = 16 + 16 = 32 bytes
	float x, y, z;
	unsigned int color; //for w/e
	float tu, tv;
	float a;//ambient
	float l;//daylight
};

class WorldBlockModel: public BlockModel
{
public:

	void Load();

	//virtual void Render();

	void BuildBlockVertices(BLOCKVERTEX* vertexArray, int blockType, byte faceInfo, int x, int y, int z);

	void BuildFaceVertices(int x, int y, int z, BLOCKVERTEX* vertexArray, int faceDir, int blockType);

	//WorldBlockModel();
	//~WorldBlockModel();
};
#endif
