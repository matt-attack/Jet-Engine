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
	Matrix3x4* OutFrames;
	CIndexBuffer* ib;
	int primitives;
	int num_indices;
	CVertexBuffer* vb;
};

//contains per instance variables to go along with a material kindof a hack atm to work for damage textures
struct MaterialInstanceBlock
{
	CTexture* extra;//well, this works for now...
};


class Renderable;
struct RenderCommand
{
	IMaterial* material;
	MaterialInstanceBlock material_instance;
	RMesh mesh;
	Renderable* source;//used to get positioning data
	bool alpha;//use this for flags
	float dist;//for sorting

	//for lighting, a bit o a hack
	Vec3 position;
	float radius;
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

//used for a basic mesh renderable
class BasicRenderable: public Renderable
{
public:
	unsigned int vcount;

	float light;
	float depthoverride;

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

#endif