#ifndef OBJMODEL_HEADER
#define OBJMODEL_HEADER

#include "../IModel.h"
#include "../../Defines.h"

#pragma pack(push)
#pragma pack(1)
struct MdlVert
{
	float x,y,z;
	float nx,ny,nz;
	float u,v;
	unsigned char blendweight[4];
	unsigned char blendindex[4];
};

struct MdlVert2
{
	float x,y,z;
	float nx,ny,nz;
	float tx,ty,tz;
	float u,v;
	unsigned char blendweight[4];
	unsigned char blendindex[4];
};
#pragma pack(pop)

void Animate(ModelData* m, JointTransform* JointTransforms, Matrix3x4* Outframes, Animation* anim, int frame1, int frame2, float blend);

class CEntity;
//remove animation stuff from ObjModel and move to other file
class ObjModel: public Renderable
{
	friend class Renderer;
	bool loaded;
	bool _external;
	Matrix3x4* OutFrames;
	JointTransform* JointTransforms;

public:
	CTexture* damage_texture = 0;
	IMaterial** mesh_materials;
	CVertexBuffer* decals;

	bool animate;
	Animation* animation;
	char* name;

	unsigned int color = 0;

	const ModelData* data;

	ObjModel();
	~ObjModel();

	void Load(const char* name, Matrix3x4* frames = 0, JointTransform* transforms = 0);//loads a model by name

	Animation* old;
	float blend; unsigned int oldanim_start, anim_start;

	float frame1, frame2;//one for each animation
	void SetAnimation(char* name);

	int GetBone(const char* name);
	Matrix3x4 GetBoneMat(const char* name);

	void UpdateAnimations();

	//updating animation data
	void Animate(Animation* anim, float frame);
	void Animate(Animation* anim, int frame1, int frame2, float blend);
	void BlendAnimate(Animation* anim1, float curframe, Animation* anim2, float curframe2, float blend);//slerps/lerps between anim 1 and anim2
	void BlendAnimate(Animation* anim1, int frame1, int frame2, float curframe, Animation* anim2, int nframe1, int nframe2, float curframe2, float blend);//slerps/lerps between anim 1 and anim2

	void DoDecal(Vec3 start, Vec3 dir, Vec3 size);

	//render debug info
	void DebugRender(CRenderer* render);
#ifndef MATT_SERVER
	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue);
#endif

	//gets a 512x512 2d texture showing the local position and bone of each surface of the model
	//this is useful for doing, for instance, spatial checks against part of the model
	//to implement something like decals
	CTexture* GetPositionMap();

private:
	CTexture* position_map = 0;
};

#endif