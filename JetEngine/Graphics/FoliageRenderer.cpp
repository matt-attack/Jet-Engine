#include "FoliageRenderer.h"
#include "RenderTexture.h"
#include "Models/ObjModel.h"
#include "../ModelData.h"
#include "ParticleRenderer.h"

#include <D3D11.h>

#include "Shader.h"
#include "CRenderer.h"
#include "../ResourceManager.h"
#include "CTexture.h"
#include "../Util/Profile.h"

FoliageRenderer::FoliageRenderer()
{
	this->shader = 0;
	this->texture = 0;
	this->data = 0;
}

FoliageRenderer::~FoliageRenderer()
{
}

Vec2 FoliageRenderer::GetImpostorSize(ObjModel* model)
{
	//ObjModel model;// = new ObjModel;
	//model.Load(name);// mech2.iqm");
	model->animate = true;//uses keyframe number for bounds
	model->aabb = model->t->joints[0].bb;
	//model.aabb.min = Vec3(model.t->bounds[25].bbmin);// -Vec3(2, 0, 10);
	//model.aabb.max = Vec3(model.t->bounds[25].bbmax);// +Vec3(2, 0, -10);

	auto ii = model;
	Matrix4 view = Matrix4::BuildMatrix(Vec3(0, 0, 1), Vec3(1, 0, 0), Vec3(0, 1, 0));// cam._matrix.LookAtLHMatrix(cam._pos, Vec3(0, 0, 0), Vec3(0, 1, 0));//cam._matrix;
	float objsNear = FLT_MAX, objsFar = -FLT_MAX;
	float objsXmin = FLT_MAX;
	float objsXmax = -FLT_MAX;
	float objsYmin = FLT_MAX;
	float objsYmax = -FLT_MAX;
	Vec3 boxCorners[6];
	boxCorners[0] = view*ii->aabb.max;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
	boxCorners[1] = view*ii->aabb.min;//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
	boxCorners[2] = view*Vec3(ii->aabb.min.x, ii->aabb.max.y, ii->aabb.max.z);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[3] = view*Vec3(ii->aabb.min.x, ii->aabb.min.y, ii->aabb.max.z);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
	boxCorners[4] = view*Vec3(ii->aabb.max.x, ii->aabb.min.y, ii->aabb.max.z);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
	boxCorners[5] = view*Vec3(ii->aabb.min.x, ii->aabb.max.y, ii->aabb.min.z);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z);

	//TransformArray(boxCorners, _countof(boxCorners), view);
	for (unsigned int corner_i = 0; corner_i < 6; corner_i++)
	{
		if (boxCorners[corner_i].z < objsNear) objsNear = boxCorners[corner_i].z;
		if (boxCorners[corner_i].z > objsFar) objsFar = boxCorners[corner_i].z;
		if (boxCorners[corner_i].x < objsXmin) objsXmin = boxCorners[corner_i].x;
		if (boxCorners[corner_i].x > objsXmax) objsXmax = boxCorners[corner_i].x;
		if (boxCorners[corner_i].y < objsYmin) objsYmin = boxCorners[corner_i].y;
		if (boxCorners[corner_i].y > objsYmax) objsYmax = boxCorners[corner_i].y;
	}

	auto dimensions = Vec2(abs(objsXmax - objsXmin), abs(objsYmax - objsYmin));
	std::swap(dimensions.x, dimensions.y);//bug fix 
	return dimensions;
}


void FoliageRenderer::Init(HeightmapTerrainSystem* system)
{
	if (this->shader)
		return;//already loaded
	
	//this->AddModel("tree.iqm");
	this->AddModel("tree2.iqm");
	//this->dimensions = this->GetImpostorSize("tree.iqm");


	//port this to use my api functions
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(TreeBillboard) * 1024;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	this->data = new TreeBillboard[1024];

	//Particle p[20];
	//ZeroMemory(&p, sizeof(Particle));
	//p.Age = 0.0f;
	//p.Type = 0;
	//experiment with forests and add tree collisions
	//	how to do groupings of trees???
	for (int i = 0; i < 1000; i++)
	{
		int model = rand() % this->tree_models.size();
		data[i].position = Vec3::random(2048, 0, 2048);// +Vec3(512, 0, 512);
		data[i].position.y = system->GetHeight(data[i].position.x, data[i].position.z);
		data[i].size = this->tree_models[model].dimensions;// Vec2(10, 20);
		data[i].position.y += data[i].size.y / 2;
		data[i].color = COLOR_ARGB(255, 255, 255, 255);
		data[i].type = model;
	}

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = data;// 0;// &p;
	vinitData.SysMemPitch = 0;
	vinitData.SysMemSlicePitch = 0;
	//renderer->device->CreateBuffer(&vbd, &vinitData, &mInitVB);

	this->num_billboards = 1000;

	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(TreeBillboard) * 1024;// mMaxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	auto res = renderer->device->CreateBuffer(&vbd, &vinitData, &mDrawVB);
	res = renderer->device->CreateBuffer(&vbd, 0, &mStreamOutVB);

	VertexElement elm9[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	//{ ELEMENT_FLOAT3, USAGE_TEXCOORD },
	{ ELEMENT_FLOAT2, USAGE_TANGENT },
	//{ ELEMENT_FLOAT2, USAGE_NORMAL },
	//{ ELEMENT_FLOAT, USAGE_BLENDWEIGHT },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT, USAGE_BLENDWEIGHT } };
	this->vd = renderer->GetVertexDeclaration(elm9, 4);

	//ok, need to make geometry shaders reloadable too

	//	maybe add some metadata in start of shader file?
	this->shader = resources.get_shader("Shaders/tree_billboards.shdr");
}

void FoliageRenderer::AddModel(const char* name)
{
	ObjModel* model = new ObjModel;
	model->Load(name);
	model->animate = true;//uses keyframe number for bounds
	model->aabb = model->t->joints[0].bb;
	this->tree_models.push_back({ this->GetImpostorSize(model), model });
}

void FoliageRenderer::Render(CRenderer* renderer, const CCamera& cam)
{
	PROFILE("particle render");

	ID3D11DeviceContext* dc = renderer->context;

	//todo: need to generate normal map in each and make sure to render at fullbright
	this->GenerateImpostors();

	float fade_distane = 150;

	//ok, now lets be super dumb
	//ok, lets speed this up considerably
	int count[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	//if (renderer->current_aa_level == 2)
	//	return;//showdebug > 1)
	for (int i = 0; i < this->num_billboards; i++)
	{
		if (this->data[i].position.distsqr(cam._pos) < fade_distane * fade_distane)
		{ 
			int type = this->data[i].type;
			auto model = this->tree_models[type];

			//make trees use the alpha test material
			//get available model
			ObjModel* rm = 0;
			if (render_models[type].size() <= count[type])
			{
				//allocate new one
				auto tmp = new ObjModel;
				tmp->Load(model.model->name);
				render_models[type].push_back(tmp);
				rm = tmp;
				count[type]++;
			}
			else
			{
				rm = render_models[type][count[type]++];
			}

			//render it
			Vec3 offset = this->data[i].position - Vec3(0, model.dimensions.y / 2, 0);
			rm->aabb = rm->t->joints[0].bb;
			rm->aabb.min *= 2;
			rm->aabb.max *= 2;
			rm->aabb.max += offset;
			rm->aabb.min += offset;
			rm->matrix = Matrix4::RotationMatrixX(-3.1415926535895f / 2.0f)*Matrix4::TranslationMatrix(offset);
			r.AddRenderable(rm);
		}
	}


	Matrix4 VP = cam._matrix*cam._projectionMatrix;// ViewProj();
	//int total = mNum;
	//
	// Set constants.
	//
	struct ConstantBufferType
	{
		Matrix4 vp;
		Vec4 eyep;
		//float padding2;
		//Vec3 lightd;
		Vec2 pos[4];
	} skyb;
	skyb.pos[0] = Vec2(0.0f, 1.0f);
	skyb.pos[1] = Vec2(1.0f, 1.0f);
	skyb.pos[2] = Vec2(0.0f, 0.0f);
	skyb.pos[3] = Vec2(1.0f, 0.0f);

	skyb.vp = VP.Transpose();
	skyb.eyep = (Vec4)cam._pos;

	renderer->SetShader(this->shader);
	renderer->SetCullmode(CULL_NONE);

	this->shader->cbuffers["Variables"].UploadAndSet(&skyb, sizeof(ConstantBufferType));

	//
	// Set IA stage.
	//
	//renderer->DepthWriteEnable(true);
	this->shader->BindIL(&this->vd);// GetVertexDeclaration(22));
	renderer->EnableAlphaBlending(false);
	renderer->context->GSSetShader(this->shader->gshader, 0, 0);
	//dc->IASetInputLayout(renderer->GetVertexDeclaration(15));
	renderer->SetPrimitiveType(PrimitiveType::PT_POINTS);
	//dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(TreeBillboard);
	UINT offset = 0;

	//this->texture = resources.get<CTexture>("smoke.png");
	renderer->SetPixelTexture(4, this->texture);
	// done streaming-out--unbind the vertex buffer
	//ID3D11Buffer* bufferArray[1] = { 0 };
	//dc->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	//std::swap(mDrawVB, mStreamOutVB);

	// Draw the updated particle system we just streamed-out. 
	dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	dc->Draw(this->num_billboards, 0);
	//dc->DrawAuto();

	renderer->context->GSSetShader(0, 0, 0);
	//renderer->EnableAlphaBlending(false);
	//renderer->DepthWriteEnable(true);

	
	//dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int rrr = 0;
void FoliageRenderer::GenerateImpostors()
{
	if (this->texture == 0)
		this->texture = CRenderTexture::Create(2048, 2048, DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT);
	else if (hack++ > 1)
		return;

	auto rt = (CRenderTexture*)this->texture;
	CCamera cam;

	auto oldrt = renderer->GetRenderTarget(0);
	renderer->SetRenderTarget(0, rt);

	Viewport old;
	renderer->GetViewport(&old);
	
	rt->Clear(0, 0, 0, 0);

	renderer->EnableAlphaBlending(false);
	
	cam._matrix = Matrix4::BuildMatrix(Vec3(0, 0, 1), Vec3(1, 0, 0), Vec3(0, 1, 0));

	if (this->tree_models.size() > 8)
		throw 7;

	//render each view
	for (int m = 0; m < this->tree_models.size(); m++)
	{
		auto model = this->tree_models[m].model;
		for (int i = 0; i < 8; i++)
		{
			//setup viewport....
			Viewport vp;
			vp.Height = 256;
			vp.Width = 256;
			vp.MinZ = 0;
			vp.MaxZ = 1;
			vp.X = 256 * i;
			vp.Y = 256 * m;// 256;
			renderer->SetViewport(&vp);

			//model.aabb.min = Vec3(model.t->bounds[25].bbmin);// -Vec3(2, 0, 10);
			//model.aabb.max = Vec3(model.t->bounds[25].bbmax);// +Vec3(2, 0, -10);
			model->matrix = Matrix4::RotationMatrixZ((3.14159265 / 4.0) *((float)i));// *Matrix4::TranslationMatrix(Vec3(0, 0, -model.aabb.min.z));


			auto ii = model;
			Matrix4 view = cam._matrix;
			float objsNear = FLT_MAX, objsFar = -FLT_MAX;
			float objsXmin = FLT_MAX;
			float objsXmax = -FLT_MAX;
			float objsYmin = FLT_MAX;
			float objsYmax = -FLT_MAX;
			Vec3 boxCorners[6];
			boxCorners[0] = view*ii->aabb.max;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
			boxCorners[1] = view*ii->aabb.min;//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
			boxCorners[2] = view*Vec3(ii->aabb.min.x, ii->aabb.max.y, ii->aabb.max.z);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[3] = view*Vec3(ii->aabb.min.x, ii->aabb.min.y, ii->aabb.max.z);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
			boxCorners[4] = view*Vec3(ii->aabb.max.x, ii->aabb.min.y, ii->aabb.max.z);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
			boxCorners[5] = view*Vec3(ii->aabb.min.x, ii->aabb.max.y, ii->aabb.min.z);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z);

			//TransformArray(boxCorners, _countof(boxCorners), view);
			for (unsigned int corner_i = 0; corner_i < 6; corner_i++)
			{
				if (boxCorners[corner_i].z < objsNear) objsNear = boxCorners[corner_i].z;
				if (boxCorners[corner_i].z > objsFar) objsFar = boxCorners[corner_i].z;
				if (boxCorners[corner_i].x < objsXmin) objsXmin = boxCorners[corner_i].x;
				if (boxCorners[corner_i].x > objsXmax) objsXmax = boxCorners[corner_i].x;
				if (boxCorners[corner_i].y < objsYmin) objsYmin = boxCorners[corner_i].y;
				if (boxCorners[corner_i].y > objsYmax) objsYmax = boxCorners[corner_i].y;
			}

			cam._projectionMatrix = Matrix4::OrthographicOffCenterLHMatrix(
				objsXmin,// left
				objsXmax,// right
				objsYmin,// bottom
				objsYmax,// top
				objsNear,// near
				objsFar).Transpose();// far

			r.Render(&cam, model);
		}
	}

	renderer->SetRenderTarget(0, &oldrt);
	renderer->SetViewport(&old);

	//todo: make this work
	renderer->context->GenerateMips(this->texture->texture);
}