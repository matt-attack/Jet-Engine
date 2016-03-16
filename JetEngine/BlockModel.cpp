#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = NULL; }

#ifndef MATT_SERVER
#include "Graphics/CRenderer.h"
#endif
#include "BlockModel.h"
#include "IMaterial.h"
//#include "Entities/PlayerEntity.h"

#define MODEL_X 32
#define MODEL_Y 32
#define MODEL_Z 32

IMaterial blockmt("textured blockmodel", "Shaders/texturedblock.shdr", Point, "texture.png", CULL_CW, false);
IMaterial blockm("blockmodel", "Shaders/blockmodel.shdr", Point, 0, CULL_CW, false);

BlockModel::BlockModel()//CWorld world, int x, int y, int z)
{
#ifndef MATT_SERVER
	this->parent = 0;
#endif
	this->dirty = false;

	this->_solidFaceCount = 0;
	this->_waterFaceCount = 0;
	this->_solidIndex = 0;
	this->_waterIndex = 0;

	this->material = 0;//&mat_generic;

	/*this->Blocks = new int**[MODEL_X];

	for (int i = 0; i < MODEL_X; ++i)
	{
	this->Blocks[i] = new int*[MODEL_Y];

	for (int j = 0; j < MODEL_Y; ++j)
	{
	this->Blocks[i][j] = new int[MODEL_Z];
	}
	}*/

	for (int x = 0; x < MODEL_X; x++)
	{
		for (int y = 0; y < MODEL_Y; y++)
		{
			for (int z = 0; z < MODEL_Z; z++)
			{
				this->Blocks[x][y][z] = 0;
			}
		}
	}
}

BlockModel::~BlockModel()
{
	/*for (int i = 0; i < MODEL_X; ++i) {
	for (int j = 0; j < MODEL_Y; ++j)
	{
	delete [] Blocks[i][j];
	}

	delete [] Blocks[i];
	}
	delete [] Blocks;*/

	//release vbuffers
	/*if (this->v_buffer != NULL)
	{
	this->v_buffer->Release();
	}*/
}

void BlockModel::Release()
{
	/*for (int i = 0; i < MODEL_X; ++i) {
	for (int j = 0; j < MODEL_Y; ++j)
	{
	delete [] Blocks[i][j];
	}

	delete [] Blocks[i];
	}
	delete [] Blocks;*/

	/*if (this->v_buffer != NULL)
	{
	this->v_buffer->Release();
	}*/
}

void BlockModel::SetPosition(int x, int y, int z)
{
	//make matrix here for translation
	//D3DXMATRIX mat, mat2;
	//D3DXMatrixTranslation(&mat, 0.0f, 50.0f, 0.0f);
	//D3DXMatrixScaling(&mat2, 10,10,10);
	//this->_matrix = *(Matrix4*)&(mat2*mat);
}

void BlockModel::AddBlock(int x, int y, int z, int blockType)
{
	this->Blocks[x][y][z] = blockType;
	this->dirty = true;
}

#define SMOOTH_LIGHTING

#define TEST//fixes lighting between regions

void BlockModel::BuildBlockVertices(VERTEX2* vertexArray, int blockType, byte faceInfo, int x, int y, int z)
{
	if (x == 0)
		BuildFaceVertices(x, y, z, vertexArray, XDecreasing, blockType);
	else if (this->Blocks[x-1][y][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, XDecreasing, blockType);
	if (x == MODEL_X - 1)
		BuildFaceVertices(x, y, z, vertexArray, XIncreasing, blockType);
	else if (this->Blocks[x+1][y][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, XIncreasing, blockType);

	if (y == 0)
		BuildFaceVertices(x, y, z, vertexArray, YDecreasing, blockType);
	else if (this->Blocks[x][y-1][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, YDecreasing, blockType);
	if (y == MODEL_Y - 1)
		BuildFaceVertices(x, y, z, vertexArray, YIncreasing, blockType);
	else if (this->Blocks[x][y+1][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, YIncreasing, blockType);

	if (z == 0)
		BuildFaceVertices(x, y, z, vertexArray, ZDecreasing, blockType);
	else if (this->Blocks[x][y][z-1] == 0)
		BuildFaceVertices(x, y, z, vertexArray, ZDecreasing, blockType);
	if (z == MODEL_Z - 1)
		BuildFaceVertices(x, y, z, vertexArray, ZIncreasing, blockType);
	else if (this->Blocks[x][y][z+1] == 0)
		BuildFaceVertices(x, y, z, vertexArray, ZIncreasing, blockType);
}

void BlockModel::BuildFaceVertices(int x, int y, int z, VERTEX2* vertexArray, int faceDir, int blockType)
{
	int index = this->_solidIndex;
	unsigned int light = blockType;//0xFF000000;

	int bid = blockType-1;
	int j = (bid & 0xf) << 4;
	int k = bid & 0xf0;
	float coordXmin = ((float)j + 0.0f * 16.0f) / 256.0f;//minX
	float coordX = (((float)j + 1.0f * 16.0f) - 0.01f) / 256.0f;//maxX
	float coordYmin = ((float)(k + 16) - 1.0f * 16.0f) / 256.0f;//maxY
	float coordY = ((float)(k + 16) - 0.0f * 16.0f - 0.01f) / 256.0f;//min y
	//float coordXmin = (blockType%16-1.0f)/16.0f;//( (float)(blockType % 16)-1)/16.0f;
	//float coordX = (blockType%16)/16.0f;//(blockType % 16)/16.0f;
	//float coordY = ((float)(blockType/16 + 1.0f))/16.0f;//(blockType /0.0625f;
	//float coordYmin = ((blockType/16))/16.0f;

	if (faceDir == XIncreasing)//fixed, correctly orientated
	{
		vertexArray[index].x = x + 1.0f;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z + 1.0f;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].a = (float)this->Blocks[x][y][z].ambient/255.0f;
		//vertexArray[index].n.x = 1.0f;
		//vertexArray[index].n.y = 0.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].a = (float)this->Blocks[x][y][z].ambient/255.0f;
		//vertexArray[index + 1].n.x = 1.0f;
		//vertexArray[index + 1].n.y = 0.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordX;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].a = (float)this->Blocks[x][y][z].ambient/255.0f;
		//vertexArray[index + 2].n.x = 1.0f;
		//vertexArray[index + 2].n.y = 0.0f;
		//vertexArray[index + 2].n.z = 0.0f;

		vertexArray[index + 3].x = x + 1.0f;
		vertexArray[index + 3].y = y;
		vertexArray[index + 3].z = z + 1.0f;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordY;
		//vertexArray[index + 3].n.x = 1.0f;
		//vertexArray[index + 3].n.y = 0.0f;
		//vertexArray[index + 3].n.z = 0.0f;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y + 1.0f;
		vertexArray[index + 4].z = z;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordYmin;
		//vertexArray[index + 4].n.x = 1.0f;
		//vertexArray[index + 4].n.y = 0.0f;
		//vertexArray[index + 4].n.z = 0.0f;

		vertexArray[index + 5].x = x + 1.0f;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 1.0f;
		//vertexArray[index + 5].n.y = 0.0f;
		//vertexArray[index + 5].n.z = 0.0f;
	}

	if (faceDir == XDecreasing)
	{
		vertexArray[index].x = x;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = -1.0f;
		//vertexArray[index].n.y = 0.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z + 1.0f;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = -1.0f;
		//vertexArray[index + 1].n.y = 0.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordXmin;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = -1.0f;
		//vertexArray[index + 2].n.y = 0.0f;
		//vertexArray[index + 2].n.z = 0.0f;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y + 1.0f;
		vertexArray[index + 3].z = z;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordYmin;
		//vertexArray[index + 3].n.x = -1.0f;
		//vertexArray[index + 3].n.y = 0.0f;
		//vertexArray[index + 3].n.z = 0.0f;

		vertexArray[index + 4].x = x;
		vertexArray[index + 4].y = y;
		vertexArray[index + 4].z = z + 1.0f;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordY;
		//vertexArray[index + 4].n.x = -1.0f;
		//vertexArray[index + 4].n.y = 0.0f;
		//vertexArray[index + 4].n.z = 0.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordX;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = -1.0f;
		//vertexArray[index + 5].n.y = 0.0f;
		//vertexArray[index + 5].n.z = 0.0f;
	}

	if (faceDir == YIncreasing)
	{
		vertexArray[index].x = x;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordXmin;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = 0.0f;
		//vertexArray[index].n.y = 1.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordX;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0.0f;
		//vertexArray[index + 1].n.y = 1.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y + 1.0f;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordX;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0;
		//vertexArray[index + 2].n.y = 1.0f;
		//vertexArray[index + 2].n.z = 0;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y + 1.0f;
		vertexArray[index + 3].z = z;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordXmin;
		vertexArray[index + 3].tv = coordYmin;
		//vertexArray[index + 3].n.x = 0;
		//vertexArray[index + 3].n.y = 1.0f;
		//vertexArray[index + 3].n.z = 0;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y + 1.0f;
		vertexArray[index + 4].z = z + 1.0f;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordX;
		vertexArray[index + 4].tv = coordY;
		//vertexArray[index + 4].n.x = 0;
		//vertexArray[index + 4].n.y = 1.0f;
		//vertexArray[index + 4].n.z = 0;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y + 1.0f;
		vertexArray[index + 5].z = z + 1.0f;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 0;
		//vertexArray[index + 5].n.y = 1.0f;
		//vertexArray[index + 5].n.z = 0;
	}

	if (faceDir == YDecreasing)
	{
		vertexArray[index].x = x + 1.0f;
		vertexArray[index].y = y;
		vertexArray[index].z = z + 1.0f;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordY;
		//vertexArray[index].n.x = 0.0f;
		//vertexArray[index].n.y = -1.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordX;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0.0f;
		//vertexArray[index + 1].n.y = -1.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordXmin;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0.0f;
		//vertexArray[index + 2].n.y = -1.0f;
		//vertexArray[index + 2].n.z = 0.0f;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y;
		vertexArray[index + 3].z = z + 1.0f;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordXmin;
		vertexArray[index + 3].tv = coordY;
		//vertexArray[index + 3].n.x = 0.0f;
		//vertexArray[index + 3].n.y = -1.0f;
		//vertexArray[index + 3].n.z = 0.0f;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y;
		vertexArray[index + 4].z = z;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordX;
		vertexArray[index + 4].tv = coordYmin;
		//vertexArray[index + 4].n.x = 0.0f;
		//vertexArray[index + 4].n.y = -1.0f;
		//vertexArray[index + 4].n.z = 0.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordYmin;
		//vertexArray[index + 5].n.x = 0.0f;
		//vertexArray[index + 5].n.y = -1.0f;
		//vertexArray[index + 5].n.z = 0.0f;
	}

	if (faceDir == ZIncreasing)
	{
		vertexArray[index].x = x;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z + 1.0f;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = 0.0f;
		//vertexArray[index].n.y = 0.0f;
		//vertexArray[index].n.z = 1.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z + 1.0f;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0;
		//vertexArray[index + 1].n.y = 0;
		//vertexArray[index + 1].n.z = 1.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordXmin;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0;
		//vertexArray[index + 2].n.y = 0;
		//vertexArray[index + 2].n.z = 1.0f;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y + 1.0f;
		vertexArray[index + 3].z = z + 1.0f;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordYmin;
		//vertexArray[index + 3].n.x = 0;
		//vertexArray[index + 3].n.y = 0;
		//vertexArray[index + 3].n.z = 1.0f;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y;
		vertexArray[index + 4].z = z + 1.0f;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordY;
		//vertexArray[index + 4].n.x = 0;
		//vertexArray[index + 4].n.y = 0;
		//vertexArray[index + 4].n.z = 1.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z + 1.0f;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordX;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 0;
		//vertexArray[index + 5].n.y = 0;
		//vertexArray[index + 5].n.z = 1.0f;
	}

	if (faceDir == ZDecreasing)
	{
		vertexArray[index].x = x + 1.0f;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = 0;
		//vertexArray[index].n.y = 0;
		//vertexArray[index].n.z = -1.0f;

		vertexArray[index + 1].x = x;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0;
		//vertexArray[index + 1].n.y = 0;
		//vertexArray[index + 1].n.z = -1.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordX;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0;
		//vertexArray[index + 2].n.y = 0;
		//vertexArray[index + 2].n.z = -1.0f;

		vertexArray[index + 3].x = x + 1.0f;
		vertexArray[index + 3].y = y;
		vertexArray[index + 3].z = z;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordY;
		//vertexArray[index + 3].n.x = 0;
		//vertexArray[index + 3].n.y = 0;
		//vertexArray[index + 3].n.z = -1.0f;

		vertexArray[index + 4].x = x;
		vertexArray[index + 4].y = y + 1.0f;
		vertexArray[index + 4].z = z;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordYmin;
		//vertexArray[index + 4].n.x = 0;
		//vertexArray[index + 4].n.y = 0;
		//vertexArray[index + 4].n.z = -1.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 0;
		//vertexArray[index + 5].n.y = 0;
		//vertexArray[index + 5].n.z = -1.0f;

		/*vertexArray[index] = new VertexPositionTextureShade(new Vector3(x + 1, y + 1, z), UVList[0], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 1] = new VertexPositionTextureShade(new Vector3(x, y + 1, z), UVList[1], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 2] = new VertexPositionTextureShade(new Vector3(x + 1, y, z), UVList[2], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 3] = new VertexPositionTextureShade(new Vector3(x + 1, y, z), UVList[3], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 4] = new VertexPositionTextureShade(new Vector3(x, y + 1, z), UVList[4], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 5] = new VertexPositionTextureShade(new Vector3(x, y, z), UVList[5], _world.Lighting.GetLight(x, y, z - 1));*/
	}

	this->_solidIndex = index + 6;
};

VERTEX2 solidVertexArray5[100000];
void BlockModel::Load(bool fast)//todo, time and optimize, also add smooth torch lights
{
#ifndef MATT_SERVER
	//heap, the extra 1000 verts prevents going outside the heap
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };
	this->v_buffer.SetVertexDeclaration(renderer->GetVertexDeclaration(elm3, 3));

	this->_solidIndex = 0;
	this->_waterIndex = 0;
	if (fast == false)
	{
		for (int x = 0; x < MODEL_X; x++)
		{
			for (int y = 0; y < MODEL_Y; y++)
			{
				for (int z = 0; z < MODEL_Z; z++)
				{
					int blockType = this->Blocks[x][y][z];
					if (blockType != 0) //0 is the code for empty
					{
						this->BuildBlockVertices(solidVertexArray5, blockType, 0, x, y, z);
					}
				}
			}
		}
	}
	else
	{
		for (int x = 0; x < MODEL_X; x++)
		{
			for (int y = 0; y < MODEL_Y; y++)
			{
				for (int z = 0; z < MODEL_Z; z++)
				{
					if (x == 31 || x == 0 || y == 31 || y == 0 || z == 0 || z == 31)
					{
						int blockType = this->Blocks[x][y][z];
						if (blockType != 0) //0 is the code for empty
						{
							this->BuildBlockVertices(solidVertexArray5, blockType, 0, x, y, z);
						}
					}
				}
			}
		}
	}

	if (_solidIndex > 0)
	{
		this->v_buffer.Data(solidVertexArray5, this->_solidIndex*sizeof(VERTEX2), sizeof(VERTEX2));
	}

	this->dirty = false;
#endif
}


#ifndef MATT_SERVER
void BlockModel::Render(CCamera* cam, std::vector<RenderCommand>* queue)
{
	if (this->dirty)
		this->Load();

	RenderCommand rc;
	rc.material_instance.extra = 0;
	rc.alpha = this->alpha;
	rc.dist = this->dist;
	rc.material = this->material;
	rc.mesh.ib = 0;
	rc.mesh.primitives = this->_solidIndex;
	rc.mesh.vb = &this->v_buffer;
	rc.mesh.OutFrames = 0;
	rc.source = this;
	rc.material = this->tex ? &blockmt : &blockm;
	queue->push_back(rc);
}
#endif

BLOCKVERTEX solidVertexArray6[100000];
void WorldBlockModel::Load()
{
#ifndef MATT_SERVER
	//heap, the extra 1000 verts prevents going outside the heap

	this->_solidIndex = 0;
	this->_waterIndex = 0;
	for (int x = 0; x < MODEL_X; x++)
	{
		for (int y = 0; y < MODEL_Y; y++)
		{
			for (int z = 0; z < MODEL_Z; z++)
			{
				int blockType = this->Blocks[x][y][z];
				if (blockType != 0) //0 is the code for empty
				{
					//if (!blockList[blockType]->isnormalcube)
					//	this->_solidIndex = blockList[blockType]->MakeMesh(solidVertexArray6, this->_solidIndex, x, y, z, 0xFFFFFFFF, 0, 0);
					//else
						this->BuildBlockVertices(solidVertexArray6, blockType, 0, x, y, z);
				}
			}
		}
	}

	if (_solidIndex > 0)
	{
		/*if (this->v_buffer != NULL)
		{
		int iNumRefs = v_buffer->Release();

		while(iNumRefs)
		{
		iNumRefs = v_buffer->Release();
		}
		this->v_buffer = NULL;
		}*/
		this->v_buffer.Data(solidVertexArray6, this->_solidIndex*sizeof(BLOCKVERTEX),sizeof(BLOCKVERTEX));
		// create a vertex buffer interface called v_buffer
		/*renderer->d3ddev->CreateVertexBuffer(this->_solidIndex*sizeof(BLOCKVERTEX),
		D3DUSAGE_WRITEONLY,
		D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1,
		D3DPOOL_DEFAULT,
		&this->v_buffer,
		NULL);

		VOID* pVoid;    // a void pointer

		// lock v_buffer and load the vertices into it
		this->v_buffer->Lock(0, 0, (void**)&pVoid, 0);
		memcpy(pVoid, solidVertexArray6, this->_solidIndex*sizeof(BLOCKVERTEX));
		this->v_buffer->Unlock();*/
	}

	this->dirty = false;
#endif
};

void WorldBlockModel::BuildBlockVertices(BLOCKVERTEX* vertexArray, int blockType, byte faceInfo, int x, int y, int z)
{
	if (x == 0)
		BuildFaceVertices(x, y, z, vertexArray, XDecreasing, blockType);
	else if (this->Blocks[x-1][y][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, XDecreasing, blockType);
	if (x == MODEL_X - 1)
		BuildFaceVertices(x, y, z, vertexArray, XIncreasing, blockType);
	else if (this->Blocks[x+1][y][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, XIncreasing, blockType);

	if (y == 0)
		BuildFaceVertices(x, y, z, vertexArray, YDecreasing, blockType);
	else if (this->Blocks[x][y-1][z] == 0 || this->Blocks[x][y-1][z] == 140)
		BuildFaceVertices(x, y, z, vertexArray, YDecreasing, blockType);
	if (y == MODEL_Y - 1)
		BuildFaceVertices(x, y, z, vertexArray, YIncreasing, blockType);
	else if (this->Blocks[x][y+1][z] == 0)
		BuildFaceVertices(x, y, z, vertexArray, YIncreasing, blockType);

	if (z == 0)
		BuildFaceVertices(x, y, z, vertexArray, ZDecreasing, blockType);
	else if (this->Blocks[x][y][z-1] == 0)
		BuildFaceVertices(x, y, z, vertexArray, ZDecreasing, blockType);
	if (z == MODEL_Z - 1)
		BuildFaceVertices(x, y, z, vertexArray, ZIncreasing, blockType);
	else if (this->Blocks[x][y][z+1] == 0)
		BuildFaceVertices(x, y, z, vertexArray, ZIncreasing, blockType);
};

void WorldBlockModel::BuildFaceVertices(int x, int y, int z, BLOCKVERTEX* vertexArray, int faceDir, int blockType)
{
	int index = this->_solidIndex;
	unsigned int light = 0xFFFFFFFF;//blockType;//0xFF000000;

	int bid = blockType-1;
	int j = (bid & 0xf) << 4;
	int k = bid & 0xf0;
	float coordXmin = ((float)j + 0.0f * 16.0f) / 256.0f;//minX
	float coordX = (((float)j + 1.0f * 16.0f) - 0.01f) / 256.0f;//maxX
	float coordYmin = ((float)(k + 16) - 1.0f * 16.0f) / 256.0f;//maxY
	float coordY = ((float)(k + 16) - 0.0f * 16.0f - 0.01f) / 256.0f;//min y
	//float coordXmin = (blockType%16-1.0f)/16.0f;//( (float)(blockType % 16)-1)/16.0f;
	//float coordX = (blockType%16)/16.0f;//(blockType % 16)/16.0f;
	//float coordY = ((float)(blockType/16 + 1.0f))/16.0f;//(blockType /0.0625f;
	//float coordYmin = ((blockType/16))/16.0f;

	for (int i = 0; i < 6; i++)
	{
		vertexArray[index+i].a = 1.0f;
		vertexArray[index+i].l = 1.0f;
	}

	if (faceDir == XIncreasing)//fixed, correctly orientated
	{
		vertexArray[index].x = x + 1.0f;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z + 1.0f;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].a = (float)this->Blocks[x][y][z].ambient/255.0f;
		//vertexArray[index].n.x = 1.0f;
		//vertexArray[index].n.y = 0.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].a = (float)this->Blocks[x][y][z].ambient/255.0f;
		//vertexArray[index + 1].n.x = 1.0f;
		//vertexArray[index + 1].n.y = 0.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordX;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].a = (float)this->Blocks[x][y][z].ambient/255.0f;
		//vertexArray[index + 2].n.x = 1.0f;
		//vertexArray[index + 2].n.y = 0.0f;
		//vertexArray[index + 2].n.z = 0.0f;

		vertexArray[index + 3].x = x + 1.0f;
		vertexArray[index + 3].y = y;
		vertexArray[index + 3].z = z + 1.0f;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordY;
		//vertexArray[index + 3].n.x = 1.0f;
		//vertexArray[index + 3].n.y = 0.0f;
		//vertexArray[index + 3].n.z = 0.0f;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y + 1.0f;
		vertexArray[index + 4].z = z;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordYmin;
		//vertexArray[index + 4].n.x = 1.0f;
		//vertexArray[index + 4].n.y = 0.0f;
		//vertexArray[index + 4].n.z = 0.0f;

		vertexArray[index + 5].x = x + 1.0f;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 1.0f;
		//vertexArray[index + 5].n.y = 0.0f;
		//vertexArray[index + 5].n.z = 0.0f;
	}

	if (faceDir == XDecreasing)
	{
		vertexArray[index].x = x;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = -1.0f;
		//vertexArray[index].n.y = 0.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z + 1.0f;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = -1.0f;
		//vertexArray[index + 1].n.y = 0.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordXmin;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = -1.0f;
		//vertexArray[index + 2].n.y = 0.0f;
		//vertexArray[index + 2].n.z = 0.0f;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y + 1.0f;
		vertexArray[index + 3].z = z;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordYmin;
		//vertexArray[index + 3].n.x = -1.0f;
		//vertexArray[index + 3].n.y = 0.0f;
		//vertexArray[index + 3].n.z = 0.0f;

		vertexArray[index + 4].x = x;
		vertexArray[index + 4].y = y;
		vertexArray[index + 4].z = z + 1.0f;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordY;
		//vertexArray[index + 4].n.x = -1.0f;
		//vertexArray[index + 4].n.y = 0.0f;
		//vertexArray[index + 4].n.z = 0.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordX;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = -1.0f;
		//vertexArray[index + 5].n.y = 0.0f;
		//vertexArray[index + 5].n.z = 0.0f;
	}

	if (faceDir == YIncreasing)
	{
		vertexArray[index].x = x;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordXmin;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = 0.0f;
		//vertexArray[index].n.y = 1.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordX;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0.0f;
		//vertexArray[index + 1].n.y = 1.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y + 1.0f;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordX;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0;
		//vertexArray[index + 2].n.y = 1.0f;
		//vertexArray[index + 2].n.z = 0;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y + 1.0f;
		vertexArray[index + 3].z = z;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordXmin;
		vertexArray[index + 3].tv = coordYmin;
		//vertexArray[index + 3].n.x = 0;
		//vertexArray[index + 3].n.y = 1.0f;
		//vertexArray[index + 3].n.z = 0;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y + 1.0f;
		vertexArray[index + 4].z = z + 1.0f;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordX;
		vertexArray[index + 4].tv = coordY;
		//vertexArray[index + 4].n.x = 0;
		//vertexArray[index + 4].n.y = 1.0f;
		//vertexArray[index + 4].n.z = 0;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y + 1.0f;
		vertexArray[index + 5].z = z + 1.0f;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 0;
		//vertexArray[index + 5].n.y = 1.0f;
		//vertexArray[index + 5].n.z = 0;
	}

	if (faceDir == YDecreasing)
	{
		vertexArray[index].x = x + 1.0f;
		vertexArray[index].y = y;
		vertexArray[index].z = z + 1.0f;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordY;
		//vertexArray[index].n.x = 0.0f;
		//vertexArray[index].n.y = -1.0f;
		//vertexArray[index].n.z = 0.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordX;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0.0f;
		//vertexArray[index + 1].n.y = -1.0f;
		//vertexArray[index + 1].n.z = 0.0f;

		vertexArray[index + 2].x = x;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordXmin;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0.0f;
		//vertexArray[index + 2].n.y = -1.0f;
		//vertexArray[index + 2].n.z = 0.0f;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y;
		vertexArray[index + 3].z = z + 1.0f;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordXmin;
		vertexArray[index + 3].tv = coordY;
		//vertexArray[index + 3].n.x = 0.0f;
		//vertexArray[index + 3].n.y = -1.0f;
		//vertexArray[index + 3].n.z = 0.0f;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y;
		vertexArray[index + 4].z = z;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordX;
		vertexArray[index + 4].tv = coordYmin;
		//vertexArray[index + 4].n.x = 0.0f;
		//vertexArray[index + 4].n.y = -1.0f;
		//vertexArray[index + 4].n.z = 0.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordYmin;
		//vertexArray[index + 5].n.x = 0.0f;
		//vertexArray[index + 5].n.y = -1.0f;
		//vertexArray[index + 5].n.z = 0.0f;
	}

	if (faceDir == ZIncreasing)
	{
		vertexArray[index].x = x;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z + 1.0f;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = 0.0f;
		//vertexArray[index].n.y = 0.0f;
		//vertexArray[index].n.z = 1.0f;

		vertexArray[index + 1].x = x + 1.0f;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z + 1.0f;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0;
		//vertexArray[index + 1].n.y = 0;
		//vertexArray[index + 1].n.z = 1.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z + 1.0f;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordXmin;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0;
		//vertexArray[index + 2].n.y = 0;
		//vertexArray[index + 2].n.z = 1.0f;

		vertexArray[index + 3].x = x;
		vertexArray[index + 3].y = y + 1.0f;
		vertexArray[index + 3].z = z + 1.0f;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordYmin;
		//vertexArray[index + 3].n.x = 0;
		//vertexArray[index + 3].n.y = 0;
		//vertexArray[index + 3].n.z = 1.0f;

		vertexArray[index + 4].x = x + 1.0f;
		vertexArray[index + 4].y = y;
		vertexArray[index + 4].z = z + 1.0f;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordY;
		//vertexArray[index + 4].n.x = 0;
		//vertexArray[index + 4].n.y = 0;
		//vertexArray[index + 4].n.z = 1.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z + 1.0f;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordX;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 0;
		//vertexArray[index + 5].n.y = 0;
		//vertexArray[index + 5].n.z = 1.0f;
	}

	if (faceDir == ZDecreasing)
	{
		vertexArray[index].x = x + 1.0f;
		vertexArray[index].y = y + 1.0f;
		vertexArray[index].z = z;
		vertexArray[index].color = light;
		vertexArray[index].tu = coordX;
		vertexArray[index].tv = coordYmin;
		//vertexArray[index].n.x = 0;
		//vertexArray[index].n.y = 0;
		//vertexArray[index].n.z = -1.0f;

		vertexArray[index + 1].x = x;
		vertexArray[index + 1].y = y + 1.0f;
		vertexArray[index + 1].z = z;
		vertexArray[index + 1].color = light;
		vertexArray[index + 1].tu = coordXmin;
		vertexArray[index + 1].tv = coordYmin;
		//vertexArray[index + 1].n.x = 0;
		//vertexArray[index + 1].n.y = 0;
		//vertexArray[index + 1].n.z = -1.0f;

		vertexArray[index + 2].x = x + 1.0f;
		vertexArray[index + 2].y = y;
		vertexArray[index + 2].z = z;
		vertexArray[index + 2].color = light;
		vertexArray[index + 2].tu = coordX;
		vertexArray[index + 2].tv = coordY;
		//vertexArray[index + 2].n.x = 0;
		//vertexArray[index + 2].n.y = 0;
		//vertexArray[index + 2].n.z = -1.0f;

		vertexArray[index + 3].x = x + 1.0f;
		vertexArray[index + 3].y = y;
		vertexArray[index + 3].z = z;
		vertexArray[index + 3].color = light;
		vertexArray[index + 3].tu = coordX;
		vertexArray[index + 3].tv = coordY;
		//vertexArray[index + 3].n.x = 0;
		//vertexArray[index + 3].n.y = 0;
		//vertexArray[index + 3].n.z = -1.0f;

		vertexArray[index + 4].x = x;
		vertexArray[index + 4].y = y + 1.0f;
		vertexArray[index + 4].z = z;
		vertexArray[index + 4].color = light;
		vertexArray[index + 4].tu = coordXmin;
		vertexArray[index + 4].tv = coordYmin;
		//vertexArray[index + 4].n.x = 0;
		//vertexArray[index + 4].n.y = 0;
		//vertexArray[index + 4].n.z = -1.0f;

		vertexArray[index + 5].x = x;
		vertexArray[index + 5].y = y;
		vertexArray[index + 5].z = z;
		vertexArray[index + 5].color = light;
		vertexArray[index + 5].tu = coordXmin;
		vertexArray[index + 5].tv = coordY;
		//vertexArray[index + 5].n.x = 0;
		//vertexArray[index + 5].n.y = 0;
		//vertexArray[index + 5].n.z = -1.0f;

		/*vertexArray[index] = new VertexPositionTextureShade(new Vector3(x + 1, y + 1, z), UVList[0], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 1] = new VertexPositionTextureShade(new Vector3(x, y + 1, z), UVList[1], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 2] = new VertexPositionTextureShade(new Vector3(x + 1, y, z), UVList[2], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 3] = new VertexPositionTextureShade(new Vector3(x + 1, y, z), UVList[3], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 4] = new VertexPositionTextureShade(new Vector3(x, y + 1, z), UVList[4], _world.Lighting.GetLight(x, y, z - 1));
		vertexArray[index + 5] = new VertexPositionTextureShade(new Vector3(x, y, z), UVList[5], _world.Lighting.GetLight(x, y, z - 1));*/
	}

	this->_solidIndex = index + 6;
};