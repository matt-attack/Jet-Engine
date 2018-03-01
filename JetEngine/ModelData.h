#pragma once

#include "ResourceManager.h"
#include "iqm.h"
#include "Math/Matrix.h"
#include "Graphics/CVertexBuffer.h"

//#include "Renderable.h"
#include "Graphics/CRenderer.h"
#include "Graphics/CIndexBuffer.h"

class IMaterial;

//this stores per instance data, do not recreate model vertex data for each, rather use a pointer and store a single instance
//elsewhere
int GetModelID(const char* str);//basically a simple hash function
#define REGISTER_MODEL(name) (*GetModelList())[GetModelID(name)] = name
std::map<int, std::string>* GetModelList();

//a model is any number of meshes that share the same vbo
//this needs reworking, should have a function to render itself (just the drawcalls)
struct Pose
{
	Quaternion rotation;
	Vec3 translation;
	Vec3 scale;
};


//all share same vertex buffer
struct Mesh
{
	CIndexBuffer* ib;

	unsigned int first_vertex, num_vertexes;
	unsigned int first_triangle, num_triangles;

	char* name;
	IMaterial* material;
};


struct Animation
{
	char* name;
	unsigned int first_frame, num_frames;
	float framerate;
	unsigned int flags;
};

struct Joint
{
	char* name;
	int parent;
	Vec3 translate;
	Vec4 rotate;
	Vec3 scale;
	//float translate[3], rotate[4], scale[3];
	Matrix3x4 matrix, invmatrix;
	AABB bb;
};

struct JointTransform
{
	bool enabled;
	Matrix3x4 transform;
};

struct Mesh;
struct Joint;
struct Animation;
struct Pose;

class CVertexBuffer;
class CIndexBuffer;

typedef struct _IqmVertex {
	float X, Y, Z;
} IqmVertex;
typedef _IqmVertex IqmNormal;

typedef struct _IqmTexCoord {
	float U, V;
} IqmTexCoord;

typedef struct _IqmTriangle {
	unsigned int Vertex[3];
} IqmTriangle;

struct ObjTriangles
{
	unsigned int Vertex[3];
	unsigned int TexCoord[3];
	unsigned int Normal[3];
};

class ModelData: public Resource
{
public:
	ModelData() {};
	~ModelData();

	std::string name;

#ifndef MATT_SERVER
	int verts;
	CVertexBuffer vb;
	CVertexBuffer vbt;//has tangents
#endif

	int NumVertex, NumNormal, NumTexCoord, NumTriangle;

	int num_meshes;
	Mesh* meshes;

	char* names;

	IqmVertex *VertexArray;
	IqmNormal *NormalArray;
	IqmNormal *TangentArray;
	IqmTexCoord *TexCoordArray;
	unsigned char *blendweights;
	unsigned char *blendindices;

	IqmTriangle *TriangleArray;
	ObjTriangles *TriangleArray2;

	iqmbounds* bounds;

	iqmbounds default_bounds;

	Animation* anims;
	int num_joints,num_frames,num_anims;
	Matrix3x4 *frames;//depreciated
	Joint* joints;

	//bones with the name starting with !
	//int num_attachments;
	//Joint* attachments;
	Pose* poses;

	int GetBone(const char* name) const;
	Animation* GetAnimation(const char* name) const;
	int GetAnimationID(char* name) const;

	static ModelData* load_as_resource(const std::string &path, ModelData* res);

	static void LoadOBJ(ModelData* o, const char* path);
	static void LoadIQM(ModelData* o, const char* path);

	//loads vertices
	static void BuildModelFromObjModel(ModelData* out, ModelData* mdl);
	static void BuildModelFromIqmModel(ModelData* out, ModelData* mdl);

	struct OutVert
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 uv;

		unsigned int blendweight;
		//unsigned char blendweight[4];
		unsigned char blendindex[4];
	};
	std::vector<OutVert> DoDecal(Joint* bone, const Vec3& direction, const Vec3& origin);
	void Reload(ResourceManager* mgr, const std::string& filename);
};