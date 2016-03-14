#ifndef _PARTICLE_HEADER
#define _PARTICLE_HEADER

#ifndef MATT_SERVER
#include "Graphics\CRenderer.h"
#endif
#include "Math\Vector.h"
#include "Defines.h"

struct Particle
{
	Vec3 pos;
	Vec3 vel;
	short id;
	short active;
	int death;
	float size;
};
class ParticleManager
{
	Particle particles[200];//two hundred at a time
public:
	ParticleManager()
	{
		for (int i = 0; i < 200; i++)
		{
			particles[i].vel = Vec3(0,0,0);
			particles[i].active = 0;
		}
	}

	void AddParticle(const Vec3 pos, const Vec3 velocity, float size, int id, float life = 1.0f)
	{
		for (int i = 0; i < 200; i++)
		{
			if (particles[i].active == 0)
			{
				//insert
				particles[i].pos = pos;
				particles[i].vel = velocity;
				particles[i].size = size;
				particles[i].id = id;
				particles[i].death = GetTickCount() + 1000*life;
				particles[i].active = 1;
				break;
			}
		}
	}

	void Render()
	{
		//renderer->SetMatrix(WORLD_MATRIX, &this->matrix);

		renderer->SetTexture(0, renderer->terrain_texture);


		renderer->EnableAlphaBlending(false);
		renderer->SetCullmode(CULL_CW);
		renderer->SetShader(4);

		float l = 1.0f;//(this->ambientlight + this->daylight) >= 1.0f ? 1.0f : (this->ambientlight + this->daylight);
		renderer->SetShaderConstantF(4,&l);

		//this->vb.Bind(sizeof(D3DVERTEX2));
		//renderer->DrawPrimitive(PT_TRIANGLELIST, 0, this->verts);

		unsigned int time = GetTickCount();
		for (int i = 0; i < 200; i++)
		{
			if (particles[i].active)
			{
				//render here using some kind of instancing

				//renderer->DrawPrimitive(PT_TRIANGLELIST, 0, this->verts);

				if (particles[i].death > time)
					particles[i].active = 0;
			}
		}
	};
};
#endif