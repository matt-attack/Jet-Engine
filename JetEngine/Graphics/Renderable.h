#ifndef RENDERABLE_HEADER
#define RENDERABLE_HEADER

#ifndef MATT_SERVER
#include "CRenderer.h"
#include "CVertexBuffer.h"
#include "CIndexBuffer.h"
#include "Shader.h"
#endif
#include "../camera.h"

#include <vector>

#include "../IMaterial.h"

class IPreRenderable
{
public:
	virtual void PreRender() = 0;
};

//a type of node for rendering
class Parent
{
public:
	Parent* parent;
	Matrix4 mat;//orientation;
	Vec3 position;

	virtual Vec3 LocalToWorld(const Vec3& vector) = 0;
	virtual Vec3 WorldToLocal(const Vec3& vector) = 0;
};

#ifndef MATT_SERVER

enum RenderableType
{
	Standard = 0,
	Skinned = 1,
	Indexed = 2,
	Custom = 3,
};

enum MeshFlags
{
	ReceivesShadows = 1,
	CastsShadows = 2,
};

struct RMesh
{
	Matrix3x4* OutFrames;
	CIndexBuffer* ib;
	int primitives;
	int num_indices;
	CVertexBuffer* vb;
};

struct MaterialInstanceBlock
{
	CTexture* extra;//well, this works for now...
};


struct RenderCommand
{
	IMaterial* material;
	MaterialInstanceBlock material_instance;
	RMesh mesh;
	Renderable* source;//used to get positioning data
	bool alpha;//use this for flags
	float dist;//for sorting
};

class CEntity;
class Renderable
{
	friend class Renderer;

	bool updated;//if the pre-render hook has been called

	//add type here, ibo or just vbo based
	//then for objects with alpha, have them get run through twice

	//maybe add a type here to make casting easy, no need for silly dynamic cast
protected:
	float dist;
public:

	bool castsShadows;
	bool receivesShadows;
	bool alpha;//used for sorting
	RenderableType type;//used for casting

	Parent* parent;

	AABB aabb;
	Matrix4 matrix;
	IMaterial* material;//use me

	CVertexBuffer* vb;

	float daylight, ambientlight;

	//decouple me from entity system later, use lambdas
	IPreRenderable* entity;//used for update hooks

	Renderable()
	{
		entity = 0;
		castsShadows = 0;
		receivesShadows = 0;
		material = 0;
		parent = 0;
		alpha = false;
		aabb.max = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		aabb.min = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		type = RenderableType::Standard;
	}
	virtual ~Renderable() {};

	//this is where you submit meshes to the queue, and call render on all of your children
	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue) {};
};

/*class EntityRenderable: public Renderable
{
CEntity* entity;
public:
EntityRenderable(CEntity* end);
~EntityRenderable();

virtual void 
};*/

class BasicRenderable: public Renderable
{
public:
	unsigned int vcount;

	float light;
	float depthoverride;

	//CVertexBuffer* vb;

	BasicRenderable()
	{
		this->depthoverride = 0.0f;
		this->alpha = false;
	}

	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue)
	{
		RenderCommand rc;
		rc.material_instance.extra = 0;
		rc.material = this->material;
		rc.alpha = this->alpha;
		rc.dist = FLT_MAX;
		rc.source = this;
		rc.mesh.ib = 0;
		rc.mesh.vb = this->vb;
		rc.mesh.primitives = vcount/3;
		queue->push_back(rc);
	}
};


extern IMaterial chunk_material;
extern IMaterial achunk_material;
class ChunkRenderable: public Renderable
{

public:
	//float light;
	//CVertexBuffer* vb;
	CVertexBuffer* water_vb;

	ChunkRenderable()
	{
		this->castsShadows = 1;
		this->receivesShadows = 1;
	}

	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue)//void Render(CRenderer* render)
	{
		if (this->vb->GetSize() > 0)
		{
			RenderCommand rc;
			rc.material_instance.extra = 0;
			rc.material = &chunk_material;
			rc.alpha = false;
			//rc.shader = 1;
			rc.dist = this->dist;
			rc.source = this;
			rc.mesh.ib = 0;
			rc.mesh.vb = this->vb;
			rc.mesh.primitives = this->vb->GetSize()/this->vb->GetStride();
			queue->push_back(rc);
		}

		if (this->water_vb->GetSize() > 0)
		{
			RenderCommand rc;
			rc.material = &achunk_material;
			rc.alpha = true;
			rc.material_instance.extra = 0;
			//rc.shader = 1;
			rc.dist = this->dist;
			rc.source = this;
			rc.mesh.ib = 0;
			rc.mesh.vb = this->water_vb;
			rc.mesh.primitives = this->water_vb->GetSize()/this->water_vb->GetStride();
			queue->push_back(rc);
		}

		//CShader* shader = render->SetShader(1);

		//render->SetTexture(0,render->terrain_texture);
		//render->SetMatrix(WORLD_MATRIX, &this->matrix);

		//shader->SetUniformF("DL", &light);

		//vb->Bind();
#ifdef USEOPENGL
		render->DrawPrimitive( PT_TRIANGLELIST, 0, vcount);
		//glDrawArrays( GL_TRIANGLES, 0, vcount );
		glBindBuffer( GL_ARRAY_BUFFER, 0);
#else
		//if (this->alpha == false)
		//	render->d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

		//int vcount = this->vb->GetSize()/this->vb->GetStride();

		//renderer->d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vcount/3);
		//render->DrawPrimitive(PT_TRIANGLELIST, 0, vcount);

		//if (this->alpha == false)
		//	render->d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
#endif
	}
};

const unsigned int SHADOW_MAP_MAX_CASCADE_COUNT = 4;//max number
const UINT SHADOW_MAP_SIZE = 1024;
//const D3DFORMAT SHADOW_MAP_FORMAT = D3DFMT_R32F;
//add settings for shadows and a settings menu/cvars

//perhaps just move to a bunch of Mesh renderables (or "draw orders") with a transform, parent, a vb, an ib, and a material
//there would be just 3 types, indexed, non indexed, and indexed+skinned
//then shaders can be autoselected/generated based on material and vertex format
//would be the "ubershader" approach but ends up being MUCH more flexible since backend does all rendering
//and you can just drop in different backends for things like deferred rendering

#include <queue>

interface ID3D11Texture2D;

class Renderer
{
	//have several lists of renderables
	std::vector<Renderable*> renderables;

	CShader *shader_s, *shader_ss;
public:
	ID3D11SamplerState* shadowSampler;
	ID3D11SamplerState* shadowSampler_linear;

	//shadowmapping stuff
	float shadowMappingSplitDepths[SHADOW_MAP_MAX_CASCADE_COUNT];
	ID3D11Texture2D *shadowMapTextures[SHADOW_MAP_MAX_CASCADE_COUNT];
	CRenderTexture* shadowMapSurfaces[SHADOW_MAP_MAX_CASCADE_COUNT];
	ID3D11ShaderResourceView* shadowMapViews[SHADOW_MAP_MAX_CASCADE_COUNT];
	Matrix4 shadowMapTexXforms[SHADOW_MAP_MAX_CASCADE_COUNT];

	int shadowSplits;
	float shadowMaxDist;
	float shadowSplitLogFactor;
	float shadowMapDepthBias;
	Vec3 dirToLight;

	void CalcShadowMapSplitDepths(float *outDepths, CCamera* camera, float maxdist);
	void CalcShadowMapMatrices(Matrix4 &outViewProj, Matrix4 &outShadowMapTexXform,CCamera* cam, std::vector<Renderable*>* objs, int id);

public:
	int rcount;
	int rdrawn;

	//ok several different types of light
	//1. directional
	//2. point directional (approximated point with no falloff)
	//3. point //not gonna be implemented for a while
	struct Light
	{
		int type;
		Vec3 position;
		Vec3 color;
		float radius;
	};
	std::vector<Light> lights;

	std::mutex todo_lock;
	std::queue<std::function<void()>> todo;

	void AddPreRenderTask(std::function<void()> in)
	{
		todo_lock.lock();
		todo.push(in);
		todo_lock.unlock();
	}

	Renderer();

	void Init(CRenderer* renderer);

	void Cleanup();

	void AddRenderable(Renderable* renderable)
	{
		renderable->updated = false;
		this->renderables.push_back(renderable);
	}

	//this renders all the given renderables from the given view
	void Render(CCamera* cam, CRenderer* render);

	//draws a single renderable
	void Render(CCamera* cam, Renderable* r);

	void RenderShadowMap(int id, std::vector<Renderable*>* objs, const Matrix4& viewProj);

	void Finish()//clear all renderables
	{
		this->renderables.clear();
	}

	bool _shadows;
	void EnableShadows(bool yn)
	{
		this->_shadows = true;
	}

	void SetMaxShadowDist(float dist)
	{
		this->shadowMaxDist = dist;
	}

	void SetShadowQuality(int q)
	{
		//do me add shadow quality settings, filters
	}

	//global shadow casting light direction
	void SetLightDirection(const Vec3& dir)
	{
		this->dirToLight = dir;
	}

	Vec3 GetLightDirection()
	{
		return this->dirToLight;
	}

private:
	//common constant buffers
	ID3D11Buffer* shadow_buffer;

	void ProcessQueue(const std::vector<RenderCommand>& renderqueue);

	inline void UpdateUniforms(const RenderCommand* rc, const CShader* shader, const Matrix4* shadowmats, bool shader_changed);

	inline void CalculateLighting();
};

extern Renderer r;

#else
class Renderable
{

};
#endif

#endif