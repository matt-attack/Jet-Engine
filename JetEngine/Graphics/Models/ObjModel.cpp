#include "ObjModel.h"
#include "../../ModelData.h"
#include "../Renderable.h"

#ifndef MATT_SERVER
#include "../CRenderer.h"
#endif

#include "../../Util/Profile.h"

ObjModel::ObjModel()
{
	this->entity = 0;
	this->animation = 0;
	this->name = 0;
	this->loaded = false;
	this->frame1 = this->frame2 = -1.0f;
	this->old = 0;
	this->_external = false;
	this->JointTransforms = 0;
	this->OutFrames = 0;
	this->animate = false;
	this->type = RenderableType::Indexed;
	this->material = 0;//&mat_generic;

	this->castsShadows = 1;
	this->receivesShadows = 1;
};

ObjModel::~ObjModel()	
{
	if (this->_external == false)
	{
		if (this->JointTransforms)
			delete[] this->JointTransforms;

		if (this->OutFrames)
			delete[] this->OutFrames;
	}

	delete[] this->mesh_materials;
};

void ObjModel::SetAnimation(char* name)
{
	if (name)
	{
		Animation* told = this->animation;
		this->animation = t->GetAnimation(name);
		if (told != this->animation)
		{
			old = told;
			oldanim_start = anim_start;
			anim_start = GetTickCount();
			blend = 0.0f;

			//new stuff
			oldanim_start = GetTickCount();
			this->frame2 = frame1;
		}
	}
	else
		this->animation = 0;
};

void ObjModel::Load(const char* name, Matrix3x4* frames, JointTransform* transforms)
{
#ifndef MATT_SERVER
	if (!loaded || strcmp(name, this->name) != 0)
	{
		if (this->_external == false)
			delete[] this->OutFrames;

		delete[] this->JointTransforms;

		t = resources.get<ModelData>(name);//renderer->GetOrLoadModel((char*)name);
		if (t->num_joints > 0)
		{
			if (frames == 0)
			{
				this->OutFrames = new Matrix3x4[t->num_joints];
				this->_external = false;
			}
			else
			{
				this->_external = true;
				this->OutFrames = frames;
			}

			if (transforms == 0)
			{
				this->JointTransforms = new JointTransform[t->num_joints];
				for (int i = 0; i < t->num_joints; i++)
				{
					this->JointTransforms[i].enabled = false;
					this->JointTransforms[i].transform = Matrix3x4(Vec4(1,0,0,0),Vec4(0,1,0,0),Vec4(0,0,1,0));//identity matrix
				}
			}
			else
				this->JointTransforms = transforms;
		}

		//setup materials
		this->mesh_materials = new IMaterial*[this->t->num_meshes];
		for (int i = 0; i < this->t->num_meshes; i++)
			this->mesh_materials[i] = this->t->meshes[i].material;

		this->vb = &this->t->vb;
		loaded = true;
		this->name = (char*)name;

		if (this->t->num_frames > 0)
			this->animate = true;
	}
#endif
};

int ObjModel::GetBone(const char* name)
{
	for (int i = 0; i < t->num_joints; i++)
	{
		if (strcmp(name,t->joints[i].name)==0)
			return i;
	}
	return -1;
};

Matrix3x4 ObjModel::GetBoneMat(const char* name)
{
	int i = 0;
	for (; i < t->num_joints; i++)
	{
		if (strcmp(name,t->joints[i].name)==0)
			break;
	}

	//compute joint matrix
	Matrix3x4 out;
	memcpy(&out, &this->OutFrames[i], sizeof(Matrix3x4));
	Matrix34Multiply_OnlySetOrigin( (float*)&this->OutFrames[i], (float*)&this->t->joints[i].matrix, (float*)&out );

	return out;
};
#ifndef MATT_SERVER
void ObjModel::BlendAnimate(Animation* anim1, int frame1, int frame2, float curframe, Animation* anim2, int oframe1, int oframe2, float curframe2, float blend)//slerps/lerps between anim 1 and anim2
{
	PROFILE("Animate");
	ModelData* m = this->t;
	if (m->num_frames <= 0) return;

	//int frame1 = (int)floor(curframe),
	//frame2 = frame1 + 1;
	//int oframe1 = (int)floor(curframe2),
	//oframe2 = oframe1 + 1;
	float frameoffset = curframe;// - frame1;
	float oframeoffset = curframe2;// - oframe1;
	//frame1 %= anim->num_frames;
	//frame2 %= anim->num_frames;
	//oframe1 %= anim2->num_frames;
	//oframe2 %= anim2->num_frames;
	frame1 += anim1->first_frame;
	frame2 += anim1->first_frame;
	oframe1 += anim2->first_frame;
	oframe2 += anim2->first_frame;

	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.

	for (int i = 0; i < m->num_joints; i++)
	{
		Quaternion q1 = Quaternion::Slerp(frameoffset, m->poses[frame1*m->num_joints + i].rotation, m->poses[frame2*m->num_joints + i].rotation, true);//m->poses[frame1*m->numjoints + i].rotation;
		Quaternion q2 = Quaternion::Slerp(oframeoffset, m->poses[oframe1*m->num_joints + i].rotation, m->poses[oframe2*m->num_joints + i].rotation, true);//m->poses[oframe1*m->numjoints + i].rotation;
		Quaternion out = Quaternion::Slerp(blend, q1, q2, true);

		Vec3 p1 = m->poses[frame1*m->num_joints + i].translation*(1-frameoffset) + m->poses[frame2*m->num_joints + i].translation*frameoffset;
		Vec3 p2 = m->poses[oframe1*m->num_joints + i].translation*(1-oframeoffset) + m->poses[oframe2*m->num_joints + i].translation*oframeoffset;
		Vec3 pout = p1*(1-blend) + p2*blend;

		Matrix3x4 mat = Matrix3x4(out, pout);

		if (this->JointTransforms[i].enabled)
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat * this->JointTransforms[i].transform;
			else this->OutFrames[i] = mat * this->JointTransforms[i].transform;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = this->JointTransforms[i].transform * mat* this->OutFrames[m->joints[i].parent];
			else this->OutFrames[i] = this->JointTransforms[i].transform*mat;
		}
		else
		{
			//if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat;
			//else this->OutFrames[i] = mat;
			if (m->joints[i].parent >= 0) this->OutFrames[i] = mat * this->OutFrames[m->joints[i].parent];// * mat;
			else this->OutFrames[i] = mat;
		}
	}
}

void ObjModel::BlendAnimate(Animation* anim, float curframe, Animation* anim2, float curframe2, float blend)
{
	PROFILE("Animate");
	ModelData* m = this->t;
	if (m->num_frames <= 0) return;

	int frame1 = (int)floor(curframe),
		frame2 = frame1 + 1;
	int oframe1 = (int)floor(curframe2),
		oframe2 = oframe1 + 1;
	float frameoffset = curframe - frame1;
	float oframeoffset = curframe2 - oframe1;
	frame1 %= anim->num_frames;
	frame2 %= anim->num_frames;
	oframe1 %= anim2->num_frames;
	oframe2 %= anim2->num_frames;
	frame1 += anim->first_frame;
	frame2 += anim->first_frame;
	oframe1 += anim2->first_frame;
	oframe2 += anim2->first_frame;

	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.

	for (int i = 0; i < m->num_joints; i++)
	{
		Quaternion q1 = Quaternion::Slerp(frameoffset, m->poses[frame1*m->num_joints + i].rotation, m->poses[frame2*m->num_joints + i].rotation, true);//m->poses[frame1*m->numjoints + i].rotation;
		Quaternion q2 = Quaternion::Slerp(oframeoffset, m->poses[oframe1*m->num_joints + i].rotation, m->poses[oframe2*m->num_joints + i].rotation, true);//m->poses[oframe1*m->numjoints + i].rotation;
		Quaternion out = Quaternion::Slerp(blend, q1, q2, true);

		Vec3 p1 = m->poses[frame1*m->num_joints + i].translation*(1-frameoffset) + m->poses[frame2*m->num_joints + i].translation*frameoffset;
		Vec3 p2 = m->poses[oframe1*m->num_joints + i].translation*(1-oframeoffset) + m->poses[oframe2*m->num_joints + i].translation*oframeoffset;
		Vec3 pout = p1*(1-blend) + p2*blend;

		Matrix3x4 mat = Matrix3x4(out, pout);

		if (this->JointTransforms[i].enabled)
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat * this->JointTransforms[i].transform;
			else this->OutFrames[i] = mat * this->JointTransforms[i].transform;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = this->JointTransforms[i].transform * mat* this->OutFrames[m->joints[i].parent];
			else this->OutFrames[i] = this->JointTransforms[i].transform*mat;
		}
		else
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat;
			else this->OutFrames[i] = mat;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = mat * this->OutFrames[m->joints[i].parent];// * mat;
			else this->OutFrames[i] = mat;

		}
	}

}

void Animate(ModelData* m, JointTransform* JointTransforms, Matrix3x4* OutFrames, Animation* anim, int frame1, int frame2, float blend)
{
	PROFILE("Animate");
	if (m->num_frames <= 0) return;

	//int frame1 = (int)floor(curframe),
	//frame2 = frame1 + 1;
	float frameoffset = blend;//curframe - frame1;
	frame1 %= anim->num_frames;
	frame2 %= anim->num_frames;
	frame1 += anim->first_frame;
	frame2 += anim->first_frame;
	Pose *pose1 = &m->poses[frame1*m->num_joints],
		*pose2 = &m->poses[frame2*m->num_joints];

	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.

	for (int i = 0; i < m->num_joints; i++)
	{
		Quaternion q = Quaternion::Slerp(frameoffset, pose1[i].rotation, pose2[i].rotation, true);//m->poses[frame1*m->numjoints + i].rotation;
		Vec3 p = pose1[i].translation*(1-frameoffset) + pose2[i].translation*frameoffset;

		Matrix3x4 mat = Matrix3x4(q, p);//oomat1*(1 - blend) + oomat2*blend;

		if (JointTransforms[i].enabled)
		{
			/*if (m->joints[i].parent >= 0) OutFrames[i] = OutFrames[m->joints[i].parent] * mat * JointTransforms[i].transform;
			else OutFrames[i] = mat * JointTransforms[i].transform;*/
			if (m->joints[i].parent >= 0) OutFrames[i] = JointTransforms[i].transform * mat* OutFrames[m->joints[i].parent];
			else OutFrames[i] = JointTransforms[i].transform*mat;
		}
		else
		{
			/*if (m->joints[i].parent >= 0) OutFrames[i] = OutFrames[m->joints[i].parent] * mat;
			else OutFrames[i] = mat;*/
			if (m->joints[i].parent >= 0) OutFrames[i] = mat * OutFrames[m->joints[i].parent];// * mat;
			else OutFrames[i] = mat;
		}
	}
};

void ObjModel::Animate(Animation* anim, int frame1, int frame2, float blend)
{
	PROFILE("Animate");
	ModelData* m = t;
	if (m->num_frames <= 0) return;

	//int frame1 = (int)floor(curframe),
	//frame2 = frame1 + 1;
	float frameoffset = blend;//curframe - frame1;
	frame1 %= anim->num_frames;
	frame2 %= anim->num_frames;
	frame1 += anim->first_frame;
	frame2 += anim->first_frame;
	Pose *pose1 = &m->poses[frame1*m->num_joints],
		*pose2 = &m->poses[frame2*m->num_joints];

	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.

	for (int i = 0; i < m->num_joints; i++)
	{
		Quaternion q = Quaternion::Slerp(frameoffset, pose1[i].rotation, pose2[i].rotation, true);//m->poses[frame1*m->numjoints + i].rotation;
		Vec3 p = pose1[i].translation*(1-frameoffset) + pose2[i].translation*frameoffset;

		Matrix3x4 mat = Matrix3x4(q, p);//oomat1*(1 - blend) + oomat2*blend;

		if (this->JointTransforms[i].enabled)
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat * this->JointTransforms[i].transform;
			else this->OutFrames[i] = mat * this->JointTransforms[i].transform;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = this->JointTransforms[i].transform * mat* this->OutFrames[m->joints[i].parent];
			else this->OutFrames[i] = this->JointTransforms[i].transform*mat;
		}
		else
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat;
			else this->OutFrames[i] = mat;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = mat * this->OutFrames[m->joints[i].parent];// * mat;
			else this->OutFrames[i] = mat;
		}
	}
};

void ObjModel::Animate(Animation* anim, float curframe)
{
	PROFILE("Animate");
	ModelData* m = t;
	if (m->num_frames <= 0) return;

	int frame1 = (int)floor(curframe),
		frame2 = frame1 + 1;
	float frameoffset = curframe - frame1;
	frame1 %= anim->num_frames;
	frame2 %= anim->num_frames;
	frame1 += anim->first_frame;
	frame2 += anim->first_frame;
	Pose *pose1 = &m->poses[frame1*m->num_joints],
		*pose2 = &m->poses[frame2*m->num_joints];

	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.

	for (int i = 0; i < m->num_joints; i++)
	{
		Quaternion q = Quaternion::Slerp(frameoffset, pose1[i].rotation, pose2[i].rotation, true);//m->poses[frame1*m->numjoints + i].rotation;
		Vec3 p = pose1[i].translation*(1-frameoffset) + pose2[i].translation*frameoffset;

		Matrix3x4 mat = Matrix3x4(q, p);//oomat1*(1 - blend) + oomat2*blend;
		//char o[500];
		//sprintf(o, "bone %s %d %d\n", m->joints[i].name, m->joints[i].parent, 0);
		//log(o);

		if (this->JointTransforms[i].enabled)
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat * this->JointTransforms[i].transform;
			else this->OutFrames[i] = mat * this->JointTransforms[i].transform;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = this->JointTransforms[i].transform * mat* this->OutFrames[m->joints[i].parent];
			else this->OutFrames[i] = this->JointTransforms[i].transform*mat;
		}
		else
		{
			/*if (m->joints[i].parent >= 0) this->OutFrames[i] = this->OutFrames[m->joints[i].parent] * mat;
			else this->OutFrames[i] = mat;*/
			if (m->joints[i].parent >= 0) this->OutFrames[i] = mat * this->OutFrames[m->joints[i].parent];// * mat;
			else this->OutFrames[i] = mat;
		}
	}
};

void ObjModel::UpdateAnimations()
{
	Animation* anim = &t->anims[0];
	if (this->animation)//just loop this
	{
		anim = this->animation;

		if (old)
		{
			float fframe1 = this->frame1;//(((float)(GetTickCount() - anim_start))/1000.0f)*anim->framerate;
			float fframe2 = this->frame2 + (((float)(GetTickCount() - oldanim_start))/1000.0f)*old->framerate;//todo, get from old frame1
			if (old->flags != IQM_LOOP)
				fframe2 = fframe2 < old->num_frames ? fframe2 : (float)old->num_frames - 0.01f;//fmod(fframe2, old->first_frame + old->num_frames);

			float blend = (((float)(GetTickCount() - anim_start))/1000.0f);//blends to new anim over 0.5 seconds, need to make adjustable
			if (blend > 1.0f)
				blend = 1.0f;
			if (blend != 1.0f)
				this->BlendAnimate(old, fframe2, anim, fframe1, blend);
			else
				this->Animate(anim, fframe1);//use cheaper blend if already transitioned
		}
		else
		{
			this->Animate(anim, this->frame1);
		}
	}
}

//IMaterial oamat("animated object", 6, Linear, 0, CULL_CW, false);
//IMaterial omat(9, Linear, 0, CULL_CW, false);

#include "../CTexture.h"
//IMaterial decal("decals", 0/*use default shader*/, Linear, "texture.png", CULL_NONE, false);

void ObjModel::Render(CCamera* cam, std::vector<RenderCommand>* queue)
{
	if (this->animate)
		this->type = RenderableType::Skinned;
	
	//todo: possible future optimization is to cache these, they dont really change
	//the i dont have to touch tons of memory just to submit
	RenderCommand rc;
	rc.material_instance.extra = this->damage_texture;
	rc.alpha = this->alpha;
	rc.dist = this->dist;
	rc.source = this;
	if (this->animate)
		rc.mesh.OutFrames = this->OutFrames;
	else
		rc.mesh.OutFrames = 0;
	rc.mesh.vb = &this->t->vb;

	for (int i = 0; i < t->num_meshes; i++)
	{
		Mesh* mesh = &t->meshes[i];
		rc.mesh.ib = mesh->ib;
		rc.mesh.num_indices = mesh->num_triangles*3;
		rc.mesh.primitives = mesh->num_vertexes;
		//ok, submit the right material for the skinning type
		//	or maybe have a skinned version for some materials?
		if (this->material)
			rc.material = this->material;
		else
			rc.material = this->mesh_materials[i];// mesh->material;

		queue->push_back(rc);
	}
	
	if (this->decals)
	{
		throw 7; //broken
		rc.mesh.ib = 0;
		this->decals->vd = rc.mesh.vb->vd;
		rc.mesh.primitives = this->decals->GetSize() / this->decals->GetStride();
		rc.mesh.vb = this->decals;
		//rc.material = &decal;

		queue->push_back(rc);
	}
	//this->DebugRender(render);
};

void ObjModel::DebugRender(CRenderer* render)
{
#ifdef _WIN32
	renderer->SetDepthRange(0.0f,0.0f);
	//compute joint matrices for rendering debug info
	Matrix3x4 jointMats[70];//should be ok
	for (int i = 0; i < t->num_joints; i++ ) 
	{
		memcpy(&jointMats[i], &this->OutFrames[i], sizeof(Matrix3x4));
		Matrix34Multiply_OnlySetOrigin( (float*)&this->OutFrames[i], (float*)&t->joints[i].matrix, (float*)&jointMats[i] );
	}

	struct vertz
	{
		Vec3 p;
		unsigned int color;
	};

	//draw bones
	auto old = renderer->current_texture;
	renderer->SetPixelTexture(0, 0);
	auto olds = renderer->shader;
	//renderer->SetShader(0);
	//renderer->d3ddev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	for (int i = 0; i < t->num_joints; i++)//yay works
	{
		//Vec3 size = t.joints[i].bb.Size()/2;
		//Vec3 m = jointMats[i].transform(Vec3(0,0,0));
		//OBB b = OBB(t.joints[i].bb, (Matrix4)this->OutFrames[i]);

		//bool picked = SphereIntersect(pos, dir, renderer->world*b.pos, b.r.length());
		//if (picked == false)
		//renderer->DrawBoundingBox(b);
		//else
		//renderer->DrawBoundingBox(b, COLOR_ARGB(255,255,0,0));
		if (t->num_frames == 0)
		{
			jointMats[i] = t->joints[i].matrix;
		}

		vertz p[6];
		Vec3 max = jointMats[i].transform(Vec3(0,0,0));
		if (t->joints[i].parent >= 0)
		{
			Vec3 min = jointMats[t->joints[i].parent].transform(Vec3(0,0,0));

			p[0].p = min;
			p[0].color = 0xFFFFFFFF;
			p[1].p = max;
			p[1].color = 0xFFFFFFFF;
			//renderer->d3ddev->DrawPrimitiveUP(D3DPT_LINELIST, 1, &p, sizeof(vertz));
		}
		//draw normals
		p[0].p = max;
		p[0].color = COLOR_ARGB(255,255,0,0);
		p[1].p = max+((Vec3)jointMats[i].a)*0.075f;
		p[1].color = COLOR_ARGB(255,255,0,0);
		p[2].p = max;
		p[2].color = COLOR_ARGB(255,0,255,0);
		p[3].p = max+((Vec3)jointMats[i].b)*0.075f;
		p[3].color = COLOR_ARGB(255,0,255,0);
		p[4].p = max;
		p[4].color = COLOR_ARGB(255,0,0,255);
		p[5].p = max+((Vec3)jointMats[i].c)*0.075f;
		p[5].color = COLOR_ARGB(255,0,0,255);
		//renderer->d3ddev->DrawPrimitiveUP(D3DPT_LINELIST, 3, &p, sizeof(vertz));
	}


	//Vec3 v = Vec3(jointMats[0].a.z,jointMats[0].b.z,jointMats[0].c.z);
	//draw bounds
	//iqmbounds* b = &m->bounds[frame2];
	//Vec3 max = (Vec3)b->bbmax;
	//Vec3 min = (Vec3)b->bbmin;
	//Matrix4 m = Matrix4::Identity();
	//Vec3 v = renderer->world.GetTranslation();
	//renderer->SetMatrix(WORLD_MATRIX, &m);
	//renderer->DrawBoundingBox(v,v+this->light_direction*5);
	//vertz p2[2];
	//p2[0].p = v;
	//p2[0].color = 0xFFFFFFFF;
	//p2[1].p = v+this->light_direction*5;
	//p2[1].color = 0xFFFFFFFF;
	//renderer->d3ddev->DrawPrimitiveUP(D3DPT_LINELIST, 1, &p2, sizeof(vertz));
	//todo render light direction and fix this crap
	//renderer->DrawBoundingBox(min,max);
	//renderer->SetTexture(0, old);
	//renderer->DepthWriteEnable(true);
	renderer->SetShader(olds);
	renderer->SetDepthRange(0.0f,1.0f);
#endif
}

#include "../RenderTexture.h"
CTexture* ObjModel::GetPositionMap()
{
	if (this->position_map)
		return this->position_map;

	//generate it
	auto crt = this->position_map ? (CRenderTexture*)this->position_map : CRenderTexture::Create(512, 512, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);

	//setup shaders to generate map
	auto shdr = resources.get<CShader>("Shaders/color_map_generator.shdr");
	renderer->SetShader(shdr);

	//bind rendertarget
	CRenderTexture rt;
	rt = renderer->GetRenderTarget(0);
	renderer->SetRenderTarget(0, crt);

	//set new viewport
	Viewport vp, oldvp;
	renderer->GetViewport(&oldvp);
	vp = oldvp;
	vp.Width = vp.Height = 512;
	renderer->SetViewport(&vp);

	crt->Clear(0, 0, 0, 0);

	//now lets bind myself
	this->t->vb.Bind();
	this->t->meshes[0].ib->Bind();

	//finally draw it
	renderer->DrawIndexedPrimitive(PrimitiveType::PT_TRIANGLELIST, 0, 0, this->t->meshes[0].num_vertexes, this->t->meshes[0].num_triangles*3);

	renderer->SetRenderTarget(0, &rt);
	renderer->SetViewport(&oldvp);

	this->position_map = crt;
	//I shouldnt have to do this, fix me later
	//this->position_map->texture = crt->GetColorResourceView();
	
	return this->position_map;
}
#endif