#ifndef _RENDERER_HEADER
#define _RENDERER_HEADER

#include "Renderable.h"

#include <mutex>

#include "../Math/Matrix.h"

#include "../HierchicalGrid.h"

#include "../ObjectPool.h"

//perhaps just move to a bunch of Mesh renderables (or "draw orders") with a transform, parent, a vb, an ib, and a material
//there would be just 3 types, indexed, non indexed, and indexed+skinned
//then shaders can be autoselected/generated based on material and vertex format
//would be the "ubershader" approach but ends up being MUCH more flexible since backend does all rendering
//and you can just drop in different backends for things like deferred rendering

const unsigned int SHADOW_MAP_MAX_CASCADE_COUNT = 4;//max number
const unsigned int SHADOW_MAP_SIZE = 1024;
//add settings for shadows and a settings menu/cvars

#include <queue>
#include <thread>

class ID3D11SamplerState;
class ID3D11Texture2D;
class ID3D11ShaderResourceView;
class ID3D11Buffer;
class CRenderTexture;
class CRenderer;
class CShader;
class IMaterial;
class LightReference;

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int count_ = 0)
		: count(count_)
	{
	}

	inline void notify() {
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		//cout << "thread " << tid << " notify" << endl;
		//notify the waiting thread
		cv.notify_one();
	}
	inline void wait() {
		std::unique_lock<std::mutex> lock(mtx);
		while (count == 0) {
			//cout << "thread " << tid << " wait" << endl;
			//wait on the mutex until notify is called
			cv.wait(lock);
			//cout << "thread " << tid << " run" << endl;
		}
		count--;
	}
private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
};

class Renderer
{
	friend class LightReference;
	friend class IMaterial;
	//have several lists of renderables
public:

	Semaphore renderable_lock_;
	Semaphore start_lock_;
private:
	
	// Shadow shaders
	CShader *shader_s, *shader_ss, *shader_sa;

	// Global light colors
	Vec3 ambient_bottom, ambient_range;
	Vec3 sun_light;

	ID3D11SamplerState* shadowSampler;
	ID3D11SamplerState* shadowSampler_linear;

	// Shadowmapping stuff
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

	void CalcShadowMapSplitDepths(float *outDepths, const CCamera* camera, float maxdist);
	void CalcShadowMapMatrices(Matrix4 &outViewProj, Matrix4 &outShadowMapTexXform, const CCamera* cam, std::vector<Renderable*>* objs, int id);

	Parent* cur_parent;

	bool shadows_;

public:
	int current_matrix;
	Matrix4 matrix_block[2000];//todo: allocate these on heap

	std::vector<Renderable*> add_renderables_, process_renderables_;

public:
	int rcount;
	int rdrawn;

	//ok several different types of light
	//1. directional (only one per scene in this case)
	//2. point directional (approximated point with no falloff)
	//3. point
	//4. spot

	enum class LightType
	{
		Point = 1,
		Spot
	};
	struct Light
	{
		LightType type;
		Vec3 position;
		Vec3 color;
		float radius;//distance for spot lights
		float angle;//in degrees for spot lights
		Vec3 direction;//for spot lights
	};

private:

	HierarchicalGrid light_grid_;
	ObjectPool<Light> light_pool_;

	std::vector<std::pair<LightReference, float>> temporary_lights_;
public:

	//need to come up with light datastructure, quadtree ? cant be searching n lights every time

	LightReference AddLight(const Light& data);

	// takes into account lifetimes, 0 = one frame only
	void AddTemporaryLight(const Light& data, float lifetime);

	Renderer();

	void Init(CRenderer* renderer);

	void Cleanup();

	/*void AddRenderable(Renderable* renderable)
	{
		renderable->updated = false;
		this->renderables.push_back(renderable);
	}*/

	// Does a threaded render 
	void ThreadedRender(CRenderer* renderer, const CCamera* cam, const Vec4& clear_color, bool notify = true);

	void GenerateQueue(const CCamera* cam, const std::vector<Renderable*>& renderable, std::vector<RenderCommand>& renderqueue);

	//this renders all the given renderables from the given view
	void Render(CCamera* cam, CRenderer* render, const std::vector<Renderable*>& renderables, bool regenerate_shadowmaps = false);

	//draws a single renderable
	void Render(CCamera* cam, Renderable* r);

	void RenderShadowMap(int id, std::vector<Renderable*>* objs, const Matrix4& viewProj);

	void EnableShadows(bool yn)
	{
		this->shadows_ = yn;
	}

	void SetMaxShadowDist(float dist)
	{
		this->shadowMaxDist = dist;
	}

	void SetShadowQuality(int q)
	{
		//do me add shadow quality settings, filters
	}

	void SetAmbient(Vec3 top, Vec3 bottom)
	{
		this->ambient_bottom = bottom;
		this->ambient_range = top - bottom;
	}

	//global shadow casting light direction
	void SetSunLightDirection(const Vec3& dir)
	{
		this->dirToLight = dir;
	}

	Vec3 GetSunLightDirection()
	{
		return this->dirToLight;
	}

	void SetSunLightColor(const Vec3& color)
	{
		this->sun_light = color;
	}

	bool rendered_shadows_ = false;
	void EndFrame();

	// things to run after rendering
	std::vector<std::function<void()>> add_queue_, process_queue_;

	// things to run before rendering
	std::vector<std::function<void()>> add_prequeue_, process_prequeue_;

private:
	//common constant buffers
	ID3D11Buffer* shadow_buffer;

	void ProcessQueue(const CCamera* cam, const std::vector<RenderCommand>& renderqueue);

	inline void UpdateUniforms(const RenderCommand* rc, const CShader* shader, 
		                       const Matrix4* shadowmats, bool shader_changed, 
		                       const Light* lights,
		                       const Matrix4* view, const Matrix4* projection);

	inline void CalculateLighting();

	inline void SetupMaterials(const RenderCommand* rc);

	inline void RenderShadowMaps(Matrix4* shadowMapViewProjs, const CCamera* cam,
		const std::vector<Renderable*>& renderables);
};

extern Renderer r;

class LightReference
{
	friend class Renderer;
	Renderer::Light* light_;
	int light_index_;

public:

	LightReference()
	{

	}

	LightReference(Renderer::Light* l, int index) :
		light_(l),
		light_index_(index)
	{

	}

	void Move(const Vec3& position, const Vec3& direction)
	{
		// remove it, move it, then add it again
		int rect[4];
		GetRect(rect);
		r.light_grid_.Remove(light_index_, rect);

		light_->position = position;
		light_->direction = direction;

		GetRect(rect);
		r.light_grid_.Insert(light_index_, rect);
	}

	void GetRect(int * rect)
	{
		if (light_->type == Renderer::LightType::Point)
		{
			rect[0] = light_->position.x - light_->radius;
			rect[1] = light_->position.z - light_->radius;
			rect[2] = light_->position.x + light_->radius;
			rect[3] = light_->position.z + light_->radius;
		}
		else
		{
			float half_radius = light_->radius * 0.5f;
			Vec3 midpoint = light_->position + light_->direction*half_radius;
			rect[0] = midpoint.x - half_radius;
			rect[1] = midpoint.z - half_radius;
			rect[2] = midpoint.x + half_radius;
			rect[3] = midpoint.z + half_radius;
		}
	}

	void Remove()
	{
		int rect[4];
		GetRect(rect);
		r.light_grid_.Remove(light_index_, rect);

		//deallocate light
#undef _free_dbg
		r.light_pool_.free(light_);
	}
};

#endif