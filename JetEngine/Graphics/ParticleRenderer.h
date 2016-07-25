
#pragma once

#include "../camera.h"
#include "CVertexBuffer.h"
#include "../ResourceManager.h"

struct Particle;
class CShader;
class ID3D11Buffer;
class CTexture;
class ID3D11DeviceContext;
class ParticleRenderer
{
	friend class ParticleSystem;
	Particle* data;
	ID3D11Buffer* mInitVB, *mDrawVB, *mStreamOutVB;
	bool mFirstRun;

	bool additive = false;
	CShader* shader;

	CTexture* texture;

	VertexDeclaration vd;

	int num_particles;
public:
	//ok, how to get different particle textures ?
	//options :
	//OK renderer for each type
	//slight cpu and batching hit
	//      good becuase number of particle types will be low
	//X one renderer using a texture array
	//     slight pixel performance hit and vertex data hit
	//X altering uv coords and using an atlas 
	//     less than optimal due to necessary geometry shader changes
	ParticleRenderer(CTexture* tex)
	{
		this->texture = tex;
		this->data = 0;
		this->shader = 0;
		this->Init();
	}

	~ParticleRenderer();


	void AddParticle(const int color, const Vec3& pos, const Vec3& vel, const Vec2& size, float lt);

	void Update(float dt);
private:
	void Init();
public:
	void Draw(ID3D11DeviceContext* dc, const CCamera& cam);
};

class ParticleSystem
{
	//int num_renderers;
	std::vector<ParticleRenderer*> renderers;
public:
	ParticleSystem()
	{
		//renderers = 0;
		//todo: add the first type of particle and sparks
	}

	~ParticleSystem()
	{
		for (auto ii : renderers)
			delete ii;
	}

	//insert new renderer and return id
	int AddParticleType(const char* texture, bool additive);

	void AddParticle(int type, const int color, const Vec3& pos, const Vec3& vel, const Vec2& size, float lt)
	{
		this->renderers[type]->AddParticle(color, pos, vel, size, lt);
	}

	void Update(float dt)
	{
		for (auto ii : this->renderers)
			ii->Update(dt);
	}

	void Draw(ID3D11DeviceContext* dc, const CCamera& cam)
	{
		for (auto ii : this->renderers)
			ii->Draw(dc, cam);
	}
};