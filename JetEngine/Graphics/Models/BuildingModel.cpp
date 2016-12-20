#include "BuildingModel.h"

#include "../CRenderer.h"
#include "../../IMaterial.h"

struct D3DVERTEX2 {
	float x, y, z;
	unsigned long color;
	float tu, tv;
};

extern IMaterial btmat;// ("bmt", "Shaders/model_diffuse.shdr", Linear, "texture.png", CULL_CW, false);
BuildingModel::BuildingModel()
{
#ifdef USEOPENGL
	this->obj = 0;
#endif
	this->vb = new CVertexBuffer(VertexBufferUsage::Static);
	this->verts = 0;
	this->castsShadows = true;
	this->material = &btmat;//&mat_point_generic;
	this->type = Standard;
}

BuildingModel::~BuildingModel()
{
	delete this->vb;
}
//todo: use terrain size as a grid to mark where buildings are
//roads in (especially in a city) can be marked as a simple graph
//to make city map look into delaunay triangulation and lloyd relaxation
//then try and join things into squares randomly
void BuildingModel::MakeCubeBuilding(float height, float x_scale, float z_scale, unsigned long color)
{
	D3DVERTEX2 vertexArray[36];// = new D3DVERTEX2[36];

	bool have_bottom = 0;
	bool have_top = 1;

	int i = 1;// blockid - 1;
	int j = (i & 0xf) << 4;
	int k = i & 0xf0;
	float coordXmin = 0;// ((float)j + 0.0f * 16.0f) / 256.0f;//minX
	float coordX = (x_scale / 40);// (((float)j + 1.0f * 16.0f) - 0.01f) / 256.0f;//maxX
	float coordYmin = 0;// ((float)(k + 16) - 1.0f * 16.0f) / 256.0f;//maxY
	float coordY = (height / 40);// ((float)(k + 16) - 0.0f * 16.0f - 0.01f) / 256.0f;//min y

	unsigned long light = color;
	float x = -0.5f*x_scale;
	float y = 0;// -0.5f*height;//0.0f;
	float z = -0.5f*z_scale;

	float scale = height;

	vertexArray[0].x = x + x_scale;
	vertexArray[0].y = y + scale;
	vertexArray[0].z = z + z_scale;
	vertexArray[0].color = light;
	vertexArray[0].tu = coordX;
	vertexArray[0].tv = coordYmin;

	vertexArray[1].x = x + x_scale;
	vertexArray[1].y = y + scale;
	vertexArray[1].z = z;
	vertexArray[1].color = light;
	vertexArray[1].tu = coordXmin;
	vertexArray[1].tv = coordYmin;

	vertexArray[2].x = x + x_scale;
	vertexArray[2].y = y;
	vertexArray[2].z = z + z_scale;
	vertexArray[2].color = light;
	vertexArray[2].tu = coordX;
	vertexArray[2].tv = coordY;

	vertexArray[3].x = x + x_scale;
	vertexArray[3].y = y;
	vertexArray[3].z = z + z_scale;
	vertexArray[3].color = light;
	vertexArray[3].tu = coordX;
	vertexArray[3].tv = coordY;

	vertexArray[4].x = x + x_scale;
	vertexArray[4].y = y + scale;
	vertexArray[4].z = z;
	vertexArray[4].color = light;
	vertexArray[4].tu = coordXmin;
	vertexArray[4].tv = coordYmin;

	vertexArray[5].x = x + x_scale;
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
	vertexArray[7].z = z + z_scale;
	vertexArray[7].color = light;
	vertexArray[7].tu = coordXmin;
	vertexArray[7].tv = coordYmin;

	vertexArray[8].x = x;
	vertexArray[8].y = y;
	vertexArray[8].z = z + z_scale;
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
	vertexArray[10].z = z + z_scale;
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

	vertexArray[13].x = x + x_scale;
	vertexArray[13].y = y + scale;
	vertexArray[13].z = z;
	vertexArray[13].color = light;
	vertexArray[13].tu = coordX;
	vertexArray[13].tv = coordYmin;

	vertexArray[14].x = x + x_scale;
	vertexArray[14].y = y + scale;
	vertexArray[14].z = z + z_scale;
	vertexArray[14].color = light;
	vertexArray[14].tu = coordX;
	vertexArray[14].tv = coordY;

	vertexArray[15].x = x;
	vertexArray[15].y = y + scale;
	vertexArray[15].z = z;
	vertexArray[15].color = light;
	vertexArray[15].tu = coordXmin;
	vertexArray[15].tv = coordYmin;

	vertexArray[16].x = x + x_scale;
	vertexArray[16].y = y + scale;
	vertexArray[16].z = z + z_scale;
	vertexArray[16].color = light;
	vertexArray[16].tu = coordX;
	vertexArray[16].tv = coordY;

	vertexArray[17].x = x;
	vertexArray[17].y = y + scale;
	vertexArray[17].z = z + z_scale;
	vertexArray[17].color = light;
	vertexArray[17].tu = coordXmin;
	vertexArray[17].tv = coordY;

	int p = 18;
	vertexArray[p].x = x;
	vertexArray[p].y = y + scale;
	vertexArray[p].z = z + z_scale;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordX;
	vertexArray[p].tv = coordYmin;

	p = 19;
	vertexArray[p].x = x + x_scale;
	vertexArray[p].y = y + scale;
	vertexArray[p].z = z + z_scale;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordXmin;
	vertexArray[p].tv = coordYmin;

	p = 20;
	vertexArray[p].x = x + x_scale;
	vertexArray[p].y = y;
	vertexArray[p].z = z + z_scale;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordXmin;
	vertexArray[p].tv = coordY;

	p = 21;
	vertexArray[p].x = x;
	vertexArray[p].y = y + scale;
	vertexArray[p].z = z + z_scale;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordX;
	vertexArray[p].tv = coordYmin;

	p = 22;
	vertexArray[p].x = x + x_scale;
	vertexArray[p].y = y;
	vertexArray[p].z = z + z_scale;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordXmin;
	vertexArray[p].tv = coordY;

	p = 23;
	vertexArray[p].x = x;
	vertexArray[p].y = y;
	vertexArray[p].z = z + z_scale;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordX;
	vertexArray[p].tv = coordY;

	p = 24;
	vertexArray[p].x = x + x_scale;
	vertexArray[p].y = y + scale;
	vertexArray[p].z = z;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordX;
	vertexArray[p].tv = coordYmin;

	p = 25;
	vertexArray[p].x = x;
	vertexArray[p].y = y + scale;
	vertexArray[p].z = z;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordXmin;
	vertexArray[p].tv = coordYmin;

	p = 26;
	vertexArray[p].x = x + x_scale;
	vertexArray[p].y = y;
	vertexArray[p].z = z;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordX;
	vertexArray[p].tv = coordY;

	p = 27;
	vertexArray[p].x = x + x_scale;
	vertexArray[p].y = y;
	vertexArray[p].z = z;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordX;
	vertexArray[p].tv = coordY;

	p = 28;
	vertexArray[p].x = x;
	vertexArray[p].y = y + scale;
	vertexArray[p].z = z;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordXmin;
	vertexArray[p].tv = coordYmin;

	p = 29;
	vertexArray[p].x = x;
	vertexArray[p].y = y;
	vertexArray[p].z = z;
	vertexArray[p].color = light;
	vertexArray[p].tu = coordXmin;
	vertexArray[p].tv = coordY;

	if (have_bottom)
	{
		p = 30;
		vertexArray[p].x = x + x_scale;
		vertexArray[p].y = y;
		vertexArray[p].z = z + z_scale;
		vertexArray[p].color = light;
		vertexArray[p].tu = coordX;
		vertexArray[p].tv = coordY;

		p = 31;
		vertexArray[p].x = x + x_scale;
		vertexArray[p].y = y;
		vertexArray[p].z = z;
		vertexArray[p].color = light;
		vertexArray[p].tu = coordX;
		vertexArray[p].tv = coordYmin;

		p = 32;
		vertexArray[p].x = x;
		vertexArray[p].y = y;
		vertexArray[p].z = z + z_scale;
		vertexArray[p].color = light;
		vertexArray[p].tu = coordXmin;
		vertexArray[p].tv = coordY;

		p = 33;
		vertexArray[p].x = x;
		vertexArray[p].y = y;
		vertexArray[p].z = z + z_scale;
		vertexArray[p].color = light;
		vertexArray[p].tu = coordXmin;
		vertexArray[p].tv = coordY;

		p = 34;
		vertexArray[p].x = x + x_scale;
		vertexArray[p].y = y;
		vertexArray[p].z = z;
		vertexArray[p].color = light;
		vertexArray[p].tu = coordX;
		vertexArray[p].tv = coordYmin;

		p = 35;
		vertexArray[p].x = x;
		vertexArray[p].y = y;
		vertexArray[p].z = z;
		vertexArray[p].color = light;
		vertexArray[p].tu = coordXmin;
		vertexArray[p].tv = coordYmin;
	}

	this->verts = 36;
	if (have_bottom == false)
		this->verts -= 6;
	if (have_top == false)
		this->verts -= 6;
#ifdef ANDROID
	if (this->obj)
	{
		glDeleteBuffers(1, &this->obj);
	}
	glGenBuffers(1, &this->obj);
	glBindBuffer(GL_ARRAY_BUFFER, this->obj);

	glBufferData(GL_ARRAY_BUFFER, sizeof(D3DVERTEX2) * 36, vertexArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
	this->vb->Data(vertexArray, sizeof(D3DVERTEX2) * this->verts, sizeof(D3DVERTEX2));
#endif
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	this->vb->SetVertexDeclaration(renderer->GetVertexDeclaration(elm3, 3));
	//delete[] vertexArray;
};

void BuildingModel::MakeFlatModel(int blockid, float scale)
{
	D3DVERTEX2 * vertexArray = new D3DVERTEX2[12];

	int i = blockid - 1;
	int j = (i & 0xf) << 4;
	int k = i & 0xf0;
	float coordXmin = ((float)j + 0.0f * 16.0f) / 256.0f;//minX
	float coordX = (((float)j + 1.0f * 16.0f) - 0.01f) / 256.0f;//maxX
	float coordYmin = ((float)(k + 16) - 1.0f * 16.0f) / 256.0f;//maxY
	float coordY = ((float)(k + 16) - 0.0f * 16.0f - 0.01f) / 256.0f;//min y

	COLOR light = COLOR_ARGB(255, 255, 255, 255);
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
	vb->Data(vertexArray, 12 * sizeof(D3DVERTEX2), sizeof(D3DVERTEX2));
#else
	glGenBuffers(1, &this->obj);
	glBindBuffer(GL_ARRAY_BUFFER, this->obj);

	glBufferData(GL_ARRAY_BUFFER, sizeof(D3DVERTEX2) * 12, vertexArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	vb->SetVertexDeclaration(renderer->GetVertexDeclaration(elm3, 3));
	delete[] vertexArray;

	this->verts = 12;
};

void BuildingModel::Render(CCamera* cam, std::vector<RenderCommand>* queue)
{
	RenderCommand rc;
	rc.alpha = this->material->alpha;
	rc.dist = this->dist;
	rc.material = this->material;
	rc.material_instance.extra = 0;
	rc.mesh.ib = 0;
	rc.mesh.primitives = this->verts;
	rc.mesh.vb = this->vb;
	rc.mesh.OutFrames = 0;
	rc.source = this;
	queue->push_back(rc);
}
