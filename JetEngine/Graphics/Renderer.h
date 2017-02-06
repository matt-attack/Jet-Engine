#ifndef _RENDERER_HEADER
#define _RENDERER_HEADER

#include "Renderable.h"

#include <mutex>

#include "../Math/Matrix.h"

//perhaps just move to a bunch of Mesh renderables (or "draw orders") with a transform, parent, a vb, an ib, and a material
//there would be just 3 types, indexed, non indexed, and indexed+skinned
//then shaders can be autoselected/generated based on material and vertex format
//would be the "ubershader" approach but ends up being MUCH more flexible since backend does all rendering
//and you can just drop in different backends for things like deferred rendering

const unsigned int SHADOW_MAP_MAX_CASCADE_COUNT = 4;//max number
const unsigned int SHADOW_MAP_SIZE = 1024;
//const D3DFORMAT SHADOW_MAP_FORMAT = D3DFMT_R32F;
//add settings for shadows and a settings menu/cvars

#include <queue>

class ID3D11SamplerState;
class ID3D11Texture2D;
class ID3D11ShaderResourceView;
class ID3D11Buffer;
class CRenderTexture;
class CRenderer;
class CShader;

class Renderer
{
	//have several lists of renderables
	std::vector<Renderable*> renderables;

	CShader *shader_s, *shader_ss, *shader_sa;
	Vec3 ambient_bottom, ambient_range;

public:
	Vec3 sun_light;
private:
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
	void CalcShadowMapMatrices(Matrix4 &outViewProj, Matrix4 &outShadowMapTexXform, CCamera* cam, std::vector<Renderable*>* objs, int id);

	std::mutex todo_lock;
	std::queue<std::function<void()>> todo;

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
		float lifetime;
	};
	std::vector<Light> lights;



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

	void SetAmbient(Vec3 top, Vec3 bottom)
	{
		this->ambient_bottom = bottom;
		this->ambient_range = top - bottom;
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

	inline void UpdateUniforms(const RenderCommand* rc, const CShader* shader, const Matrix4* shadowmats, bool shader_changed, const Light* lights);

	inline void CalculateLighting();

	inline void SetupMaterials(const RenderCommand* rc);

	inline void RenderShadowMaps(Matrix4* shadowMapViewProjs, CCamera* cam);
};

extern Renderer r;

#endif