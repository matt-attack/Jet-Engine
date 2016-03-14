#include "ParticleRenderer.h"

#include <D3D11.h>

#include "Shader.h"
#include "CRenderer.h"
#include "../ResourceManager.h"
#include "CTexture.h"

struct Particle
{
	Vec3 position, velocity;
	Vec2 size;
	float age;
	unsigned int color;
};

void ParticleRenderer::Init()
{
	this->texture = resources.get<CTexture>("smoke.png");

	if (this->shader)
		return;//already loaded

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Particle) * 1024;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	this->data = new Particle[1024];

	//Particle p[20];
	//ZeroMemory(&p, sizeof(Particle));
	//p.Age = 0.0f;
	//p.Type = 0;
	/*for (int i = 0; i < 20; i++)
	{
	p[i].position = Vec3(i * 30, i * 50, i * 20);
	p[i].velocity = Vec3(0, 0, 0);
	p[i].size = Vec2(40, 40);
	p[i].age = 10;
	}*/

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = data;// 0;// &p;
	vinitData.SysMemPitch = 0;
	vinitData.SysMemSlicePitch = 0;
	//renderer->device->CreateBuffer(&vbd, &vinitData, &mInitVB);

	this->num_particles = 200;

	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(Particle) * 1024;// mMaxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	auto res = renderer->device->CreateBuffer(&vbd, 0, &mDrawVB);
	res = renderer->device->CreateBuffer(&vbd, 0, &mStreamOutVB);

	VertexElement elm9[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_FLOAT3, USAGE_TEXCOORD },
	{ ELEMENT_FLOAT2, USAGE_TANGENT },
	//{ ELEMENT_FLOAT2, USAGE_NORMAL },
	{ ELEMENT_FLOAT, USAGE_BLENDWEIGHT },
	{ ELEMENT_COLOR, USAGE_COLOR } };
	renderer->CreateVertexDeclaration(22, elm9, 5);

	this->shader = new CShader("Content/Shaders/particles.shdr", "vs_main", "Content/Shaders/particles.shdr", "ps_main", 0, 0, "Content/Shaders/particles.shdr", "gs_main");
}

#include "../Util/Profile.h"
void ParticleRenderer::Update(float dt)
{
	PROFILE("particle update");

	//update the stufffs
	for (int i = 0; i < this->num_particles; i++)
	{
		this->data[i].age -= dt;
		if (this->data[i].age < 0)// || this->data[i].age < 0)
		{
			//this->data[i].age = 0;
			//this->data[i].position = Vec3(1024, 400, 1024);
			//this->data[i].velocity = Vec3::random(25, 25, 25);
			//this->data[i].size = Vec2(5, 5);
			this->data[i] = this->data[this->num_particles];
			this->num_particles--;
		}
		this->data[i].position = this->data[i].position + this->data[i].velocity*dt;
	}

	D3D11_MAPPED_SUBRESOURCE res;
	renderer->context->Map(mStreamOutVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, data, sizeof(Particle) * this->num_particles);
	renderer->context->Unmap(mStreamOutVB, 0);
}


void ParticleRenderer::AddParticle(const Vec3& pos, const Vec3& vel, const Vec2& size, float lt)
{
	if (this->num_particles >= 1024 - 1)
		return;

	auto p = &this->data[this->num_particles++];
	p->age = lt;
	p->position = pos;
	p->velocity = vel;
	p->size = size;
	p->color = COLOR_ARGB(255, 40, 40, 40);
}

void ParticleRenderer::Draw(ID3D11DeviceContext* dc, const CCamera& cam)
{
	PROFILE("particle render");

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
	renderer->DepthWriteEnable(false);
	this->shader->BindIL(renderer->GetVertexDeclaration(22));
	renderer->EnableAlphaBlending(true);
	renderer->context->GSSetShader(this->shader->gshader, 0, 0);
	//dc->IASetInputLayout(renderer->GetVertexDeclaration(15));
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Particle);
	UINT offset = 0;

	this->texture = resources.get<CTexture>("smoke.png");
	renderer->SetPixelTexture(4, this->texture);
	// done streaming-out--unbind the vertex buffer
	//ID3D11Buffer* bufferArray[1] = { 0 };
	//dc->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	std::swap(mDrawVB, mStreamOutVB);

	// Draw the updated particle system we just streamed-out. 
	dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	dc->Draw(this->num_particles, 0);
	//dc->DrawAuto();

	renderer->context->GSSetShader(0, 0, 0);
	renderer->EnableAlphaBlending(false);
	renderer->DepthWriteEnable(true);
}