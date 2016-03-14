#ifndef IMODEL_HEADER_
#define IMODEL_HEADER_

#include "Renderable.h"
#ifndef MATT_SERVER
#include "CRenderer.h"
#include "CVertexBuffer.h"
#include "CIndexBuffer.h"
#endif


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
	Matrix3x4 matrix,invmatrix;
	AABB bb;
};

struct JointTransform
{
	bool enabled;
	Matrix3x4 transform;
};

#endif
