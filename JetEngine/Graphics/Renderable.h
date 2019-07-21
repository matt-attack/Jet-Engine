#ifndef RENDERABLE_HEADER
#define RENDERABLE_HEADER

#include <vector>
#include <float.h>

#include "../Math/AABB.h"
#include "../Math/Matrix.h"

class CCamera;
class IMaterial;
class CVertexBuffer;
class CIndexBuffer;
class CTexture;

//used to call update methods before rendering (only called once per renderable per frame)
class IPreRenderable
{
public:
	virtual void PreRender() = 0;
};

//a type of node for rendering
class Parent;
class Parent
{
public:
	Parent* parent;
	Matrix4 mat;//orientation;
	Vec3 position;

	virtual Vec3 LocalToWorld(const Vec3& vector) = 0;
	virtual Vec3 WorldToLocal(const Vec3& vector) = 0;
};


enum RenderableType
{
	Standard = 0,
	Skinned = 1,
	Indexed = 2,
	Custom = 3,
};

//all the main details required to render a mesh's vertex or index data
struct RMesh
{
	// todo maybe add a pointer to skinning data including these two
	int num_frames;
	Matrix3x4* skinning_frames;

	CIndexBuffer* ib;
	int primitives;
	int num_indices;
	const CVertexBuffer* vb;
};

//contains per instance variables to go along with a material kindof a hack atm to work for damage textures
struct MaterialInstanceBlock
{
	CTexture* extra = 0;//well, this works for now...
	CTexture* extra2 = 0;
	unsigned int color = 0;
};


class Renderable;
class RenderCommand
{
public:
	IMaterial* material;
	MaterialInstanceBlock material_instance;
	RMesh mesh;
	Matrix4* transform;
	bool alpha;//use this for flags
	float dist;//for sorting

	//for lighting, a bit o a hack
	Vec3 position;
	float radius;

//private:
	Renderable* source;// for debugging only
};

class CEntity;
class Renderable
{
	friend class Renderer;

public:
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
	virtual void Render(const CCamera* cam, std::vector<RenderCommand>* queue) {};
};

//used for a basic mesh renderable
#include "CVertexBuffer.h"
class BasicRenderable: public Renderable
{
public:
	unsigned int vcount;

	float light;
	float depthoverride;
	float radius;

	CVertexBuffer my_vb;

	BasicRenderable()
	{
		this->depthoverride = 0.0f;
		this->alpha = false;
	}

	struct EzVert
	{
		Vec3 pos;
		Vec3 normal;
		Vec3 tangent;
		float u, v;
	};

	static void AddRect(std::vector<EzVert>& a, const Vec3& top_left, const Vec3& right, const Vec3& down)
	{
		const Vec3 normal = right.cross(down).getnormal();

		EzVert vert;
		vert.normal = normal;
		vert.tangent = Vec3(0, 0, 0);

		vert.pos = top_left;
		vert.u = 0.0f;
		vert.v = 0.0f;
		a.push_back(vert);

		vert.pos = top_left + right;
		vert.u = 1.0f;
		vert.v = 0.0f;
		a.push_back(vert);

		vert.pos = top_left + down;
		vert.u = 0.0f;
		vert.v = 1.0f;
		a.push_back(vert);

		vert.pos = top_left + right + down;
		vert.u = 1.0f;
		vert.v = 1.0f;
		a.push_back(vert);
	}

	void SetMeshEasy(const std::string& material_name, const std::string& image_name, const EzVert* vertex, int count);

	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue)
	{
		RenderCommand rc;
		rc.material_instance.extra = 0;
		rc.material_instance.extra2 = 0;
		rc.material = this->material;
		rc.alpha = this->alpha;
		rc.dist = FLT_MAX;
		rc.source = this;
		rc.mesh.ib = 0;
		rc.mesh.vb = this->vb;
		rc.mesh.primitives = vcount/3;
		rc.position = this->matrix.GetTranslation();
		rc.transform = &this->matrix;
		rc.radius = this->radius;
		queue->push_back(rc);
	}
};

#endif