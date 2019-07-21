#include "BasicModel.h"

#include "../CRenderer.h"
#include "../../IMaterial.h"

struct D3DVERTEX2 {
	float x, y, z;
	unsigned long color;
	float tu, tv;
};

IMaterial btmat("bmt", "Shaders/model_diffuse.shdr", Linear, "texture.png", CULL_CW, false);
BasicModel::BasicModel()
{
#ifdef USEOPENGL
	this->obj = 0;
#endif
	this->vb = new CVertexBuffer(VertexBufferUsage::Static);
	this->verts = 0;
	this->material = &btmat;//&mat_point_generic;
	this->type = Standard;
}

BasicModel::~BasicModel() 
{
	delete this->vb;
}

void BasicModel::MakeCubeModel(int blockid, float scale, unsigned long color)
{
	D3DVERTEX2 vertexArray[36];// = new D3DVERTEX2[36];

	int i = blockid-1;
	int j = (i & 0xf) << 4;
	int k = i & 0xf0;
	float coordXmin = ((float)j + 0.0f * 16.0f) / 256.0f;//minX
	float coordX = (((float)j + 1.0f * 16.0f) - 0.01f) / 256.0f;//maxX
	float coordYmin = ((float)(k + 16) - 1.0f * 16.0f) / 256.0f;//maxY
	float coordY = ((float)(k + 16) - 0.0f * 16.0f - 0.01f) / 256.0f;//min y

	unsigned long light = color;
	float x = -0.5f*scale;
	float y = -0.5f*scale;//0.0f;
	float z = -0.5f*scale;

	vertexArray[0].x = x + scale;
	vertexArray[0].y = y + scale;
	vertexArray[0].z = z + scale;
	vertexArray[0].color = light;
	vertexArray[0].tu = coordX;
	vertexArray[0].tv = coordYmin;

	vertexArray[1].x = x + scale;
	vertexArray[1].y = y + scale;
	vertexArray[1].z = z;
	vertexArray[1].color = light;
	vertexArray[1].tu = coordXmin;
	vertexArray[1].tv = coordYmin;

	vertexArray[2].x = x + scale;
	vertexArray[2].y = y;
	vertexArray[2].z = z + scale;
	vertexArray[2].color = light;
	vertexArray[2].tu = coordX;
	vertexArray[2].tv = coordY;

	vertexArray[3].x = x + scale;
	vertexArray[3].y = y;
	vertexArray[3].z = z + scale;
	vertexArray[3].color = light;
	vertexArray[3].tu = coordX;
	vertexArray[3].tv = coordY;

	vertexArray[4].x = x + scale;
	vertexArray[4].y = y + scale;
	vertexArray[4].z = z;
	vertexArray[4].color = light;
	vertexArray[4].tu = coordXmin;
	vertexArray[4].tv = coordYmin;

	vertexArray[5].x = x + scale;
	vertexArray[5].y = y;
	vertexArray[5].z = z;
	vertexArray[5].color = light;
	vertexArray[5].tu = coordXmin;
	vertexArray[5].tv = coordY;

	vertexArray[6].x = x;
	vertexArray[6].y = y + scale;
	vertexArray[6].z = z;
	vertexArray[6].color = light;
	vertexArray[6].tu = coordX;
	vertexArray[6].tv = coordYmin;

	vertexArray[7].x = x;
	vertexArray[7].y = y + scale;
	vertexArray[7].z = z + scale;
	vertexArray[7].color = light;
	vertexArray[7].tu = coordXmin;
	vertexArray[7].tv = coordYmin;

	vertexArray[8].x = x;
	vertexArray[8].y = y;
	vertexArray[8].z = z + scale;
	vertexArray[8].color = light;
	vertexArray[8].tu = coordXmin;
	vertexArray[8].tv = coordY;

	vertexArray[9].x = x;
	vertexArray[9].y = y + scale;
	vertexArray[9].z = z;
	vertexArray[9].color = light;
	vertexArray[9].tu = coordX;
	vertexArray[9].tv = coordYmin;

	vertexArray[10].x = x;
	vertexArray[10].y = y;
	vertexArray[10].z = z + scale;
	vertexArray[10].color = light;
	vertexArray[10].tu = coordXmin;
	vertexArray[10].tv = coordY;

	vertexArray[11].x = x;
	vertexArray[11].y = y;
	vertexArray[11].z = z;
	vertexArray[11].color = light;
	vertexArray[11].tu = coordX;
	vertexArray[11].tv = coordY;

	vertexArray[12].x = x;
	vertexArray[12].y = y + scale;
	vertexArray[12].z = z;
	vertexArray[12].color = light;
	vertexArray[12].tu = coordXmin;
	vertexArray[12].tv = coordYmin;

	vertexArray[13].x = x + scale;
	vertexArray[13].y = y + scale;
	vertexArray[13].z = z;
	vertexArray[13].color = light;
	vertexArray[13].tu = coordX;
	vertexArray[13].tv = coordYmin;

	vertexArray[14].x = x + scale;
	vertexArray[14].y = y + scale;
	vertexArray[14].z = z + scale;
	vertexArray[14].color = light;
	vertexArray[14].tu = coordX;
	vertexArray[14].tv = coordY;

	vertexArray[15].x = x;
	vertexArray[15].y = y + scale;
	vertexArray[15].z = z;
	vertexArray[15].color = light;
	vertexArray[15].tu = coordXmin;
	vertexArray[15].tv = coordYmin;

	vertexArray[16].x = x + scale;
	vertexArray[16].y = y + scale;
	vertexArray[16].z = z + scale;
	vertexArray[16].color = light;
	vertexArray[16].tu = coordX;
	vertexArray[16].tv = coordY;

	vertexArray[17].x = x;
	vertexArray[17].y = y + scale;
	vertexArray[17].z = z + scale;
	vertexArray[17].color = light;
	vertexArray[17].tu = coordXmin;
	vertexArray[17].tv = coordY;

	vertexArray[18].x = x + scale;
	vertexArray[18].y = y;
	vertexArray[18].z = z + scale;
	vertexArray[18].color = light;
	vertexArray[18].tu = coordX;
	vertexArray[18].tv = coordY;

	vertexArray[19].x = x + scale;
	vertexArray[19].y = y;
	vertexArray[19].z = z;
	vertexArray[19].color = light;
	vertexArray[19].tu = coordX;
	vertexArray[19].tv = coordYmin;

	vertexArray[20].x = x;
	vertexArray[20].y = y;
	vertexArray[20].z = z + scale;
	vertexArray[20].color = light;
	vertexArray[20].tu = coordXmin;
	vertexArray[20].tv = coordY;

	vertexArray[21].x = x;
	vertexArray[21].y = y;
	vertexArray[21].z = z + scale;
	vertexArray[21].color = light;
	vertexArray[21].tu = coordXmin;
	vertexArray[21].tv = coordY;

	vertexArray[22].x = x + scale;
	vertexArray[22].y = y;
	vertexArray[22].z = z;
	vertexArray[22].color = light;
	vertexArray[22].tu = coordX;
	vertexArray[22].tv = coordYmin;

	vertexArray[23].x = x;
	vertexArray[23].y = y;
	vertexArray[23].z = z;
	vertexArray[23].color = light;
	vertexArray[23].tu = coordXmin;
	vertexArray[23].tv = coordYmin;

	vertexArray[24].x = x;
	vertexArray[24].y = y + scale;
	vertexArray[24].z = z + scale;
	vertexArray[24].color = light;
	vertexArray[24].tu = coordX;
	vertexArray[24].tv = coordYmin;

	vertexArray[25].x = x + scale;
	vertexArray[25].y = y + scale;
	vertexArray[25].z = z + scale;
	vertexArray[25].color = light;
	vertexArray[25].tu = coordXmin;
	vertexArray[25].tv = coordYmin;

	vertexArray[26].x = x + scale;
	vertexArray[26].y = y;
	vertexArray[26].z = z + scale;
	vertexArray[26].color = light;
	vertexArray[26].tu = coordXmin;
	vertexArray[26].tv = coordY;

	vertexArray[27].x = x;
	vertexArray[27].y = y + scale;
	vertexArray[27].z = z + scale;
	vertexArray[27].color = light;
	vertexArray[27].tu = coordX;
	vertexArray[27].tv = coordYmin;

	vertexArray[28].x = x + scale;
	vertexArray[28].y = y;
	vertexArray[28].z = z + scale;
	vertexArray[28].color = light;
	vertexArray[28].tu = coordXmin;
	vertexArray[28].tv = coordY;

	vertexArray[29].x = x;
	vertexArray[29].y = y;
	vertexArray[29].z = z + scale;
	vertexArray[29].color = light;
	vertexArray[29].tu = coordX;
	vertexArray[29].tv = coordY;

	vertexArray[30].x = x + scale;
	vertexArray[30].y = y + scale;
	vertexArray[30].z = z;
	vertexArray[30].color = light;
	vertexArray[30].tu = coordX;
	vertexArray[30].tv = coordYmin;

	vertexArray[31].x = x;
	vertexArray[31].y = y + scale;
	vertexArray[31].z = z;
	vertexArray[31].color = light;
	vertexArray[31].tu = coordXmin;
	vertexArray[31].tv = coordYmin;

	vertexArray[32].x = x + scale;
	vertexArray[32].y = y;
	vertexArray[32].z = z;
	vertexArray[32].color = light;
	vertexArray[32].tu = coordX;
	vertexArray[32].tv = coordY;

	vertexArray[33].x = x + scale;
	vertexArray[33].y = y;
	vertexArray[33].z = z;
	vertexArray[33].color = light;
	vertexArray[33].tu = coordX;
	vertexArray[33].tv = coordY;

	vertexArray[34].x = x;
	vertexArray[34].y = y + scale;
	vertexArray[34].z = z;
	vertexArray[34].color = light;
	vertexArray[34].tu = coordXmin;
	vertexArray[34].tv = coordYmin;

	vertexArray[35].x = x;
	vertexArray[35].y = y;
	vertexArray[35].z = z;
	vertexArray[35].color = light;
	vertexArray[35].tu = coordXmin;
	vertexArray[35].tv = coordY;

#ifdef ANDROID
	if (this->obj)
	{
		glDeleteBuffers(1, &this->obj);
	}
	glGenBuffers( 1, &this->obj );
	glBindBuffer( GL_ARRAY_BUFFER, this->obj );

	glBufferData( GL_ARRAY_BUFFER, sizeof(D3DVERTEX2)*36, vertexArray, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0);
#else
	this->vb->Data(vertexArray, sizeof(D3DVERTEX2)*36, sizeof(D3DVERTEX2));
#endif
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	this->vb->SetVertexDeclaration(renderer->GetVertexDeclaration(elm3,3));
	//delete[] vertexArray;

	this->verts = 36;
};

void BasicModel::MakeFlatModel(int blockid, float scale)
{
	D3DVERTEX2 * vertexArray = new D3DVERTEX2[12];

	int i = blockid-1;
	int j = (i & 0xf) << 4;
	int k = i & 0xf0;
	float coordXmin = ((float)j + 0.0f * 16.0f) / 256.0f;//minX
	float coordX = (((float)j + 1.0f * 16.0f) - 0.01f) / 256.0f;//maxX
	float coordYmin = ((float)(k + 16) - 1.0f * 16.0f) / 256.0f;//maxY
	float coordY = ((float)(k + 16) - 0.0f * 16.0f - 0.01f) / 256.0f;//min y

	COLOR light = COLOR_ARGB(255,255,255,255);
	float x = -0.5f*scale;
	float y = 0.0f;
	float z = -0.5f*scale;

	vertexArray[0].x = x;// + scale;
	vertexArray[0].y = y + scale;
	vertexArray[0].z = z + scale;
	vertexArray[0].color = light;
	vertexArray[0].tu = coordX;
	vertexArray[0].tv = coordYmin;

	vertexArray[1].x = x;// + scale;
	vertexArray[1].y = y + scale;
	vertexArray[1].z = z;
	vertexArray[1].color = light;
	vertexArray[1].tu = coordXmin;
	vertexArray[1].tv = coordYmin;

	vertexArray[2].x = x;// + scale;
	vertexArray[2].y = y;
	vertexArray[2].z = z + scale;
	vertexArray[2].color = light;
	vertexArray[2].tu = coordX;
	vertexArray[2].tv = coordY;

	vertexArray[3].x = x;// + scale;
	vertexArray[3].y = y;
	vertexArray[3].z = z + scale;
	vertexArray[3].color = light;
	vertexArray[3].tu = coordX;
	vertexArray[3].tv = coordY;

	vertexArray[4].x = x;// + scale;
	vertexArray[4].y = y + scale;
	vertexArray[4].z = z;
	vertexArray[4].color = light;
	vertexArray[4].tu = coordXmin;
	vertexArray[4].tv = coordYmin;

	vertexArray[5].x = x;// + scale;
	vertexArray[5].y = y;
	vertexArray[5].z = z;
	vertexArray[5].color = light;
	vertexArray[5].tu = coordXmin;
	vertexArray[5].tv = coordY;

	vertexArray[6].x = x;
	vertexArray[6].y = y + scale;
	vertexArray[6].z = z;
	vertexArray[6].color = light;
	vertexArray[6].tu = coordX;
	vertexArray[6].tv = coordYmin;

	vertexArray[7].x = x;
	vertexArray[7].y = y + scale;
	vertexArray[7].z = z + scale;
	vertexArray[7].color = light;
	vertexArray[7].tu = coordXmin;
	vertexArray[7].tv = coordYmin;

	vertexArray[8].x = x;
	vertexArray[8].y = y;
	vertexArray[8].z = z + scale;
	vertexArray[8].color = light;
	vertexArray[8].tu = coordXmin;
	vertexArray[8].tv = coordY;

	vertexArray[9].x = x;
	vertexArray[9].y = y + scale;
	vertexArray[9].z = z;
	vertexArray[9].color = light;
	vertexArray[9].tu = coordX;
	vertexArray[9].tv = coordYmin;

	vertexArray[10].x = x;
	vertexArray[10].y = y;
	vertexArray[10].z = z + scale;
	vertexArray[10].color = light;
	vertexArray[10].tu = coordXmin;
	vertexArray[10].tv = coordY;

	vertexArray[11].x = x;
	vertexArray[11].y = y;
	vertexArray[11].z = z;
	vertexArray[11].color = light;
	vertexArray[11].tu = coordX;
	vertexArray[11].tv = coordY;

#ifndef ANDROID
	vb->Data(vertexArray, 12*sizeof(D3DVERTEX2), sizeof(D3DVERTEX2));
#else
	glGenBuffers( 1, &this->obj );
	glBindBuffer( GL_ARRAY_BUFFER, this->obj );

	glBufferData( GL_ARRAY_BUFFER, sizeof(D3DVERTEX2)*12, vertexArray, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0);
#endif
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	vb->SetVertexDeclaration(renderer->GetVertexDeclaration(elm3,3));
	delete[] vertexArray;

	this->verts = 12;
};

//this must be implemented by every model type
//IMaterial btmat(4, Linear, 0, CULL_CW, false);
//IMaterial bmat(5, Linear, 0, CULL_CW, false);
//IMaterial bamat(5, Linear, 0, CULL_CW, true);

void BasicModel::Render(const CCamera* cam, std::vector<RenderCommand>* queue)
{
	RenderCommand rc;
	rc.alpha = this->material->alpha;
	rc.dist = this->dist;
	rc.material = this->material;
	rc.material_instance.extra = 0;
	rc.mesh.ib = 0;
	rc.mesh.primitives = this->verts;
	rc.mesh.vb = this->vb;
	rc.mesh.skinning_frames = 0;
	rc.source = this;
	queue->push_back(rc);
}
