
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
	Particle* data;
	ID3D11Buffer* mInitVB, *mDrawVB, *mStreamOutVB;
	bool mFirstRun;

	CShader* shader;

	CTexture* texture;

	VertexDeclaration vd;

	int num_particles;
public:

	ParticleRenderer()
	{
		this->data = 0;
		this->shader = 0;
	}

	~ParticleRenderer();


	void AddParticle(const Vec3& pos, const Vec3& vel, const Vec2& size, float lt);

	void Update(float dt);

	void Init();

	void Draw(ID3D11DeviceContext* dc, const CCamera& cam);
};