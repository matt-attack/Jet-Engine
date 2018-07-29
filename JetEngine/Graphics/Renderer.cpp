#include "Renderer.h"
#include "CRenderer.h"
#include "../Util/Profile.h"

#include "../IMaterial.h"

#include "../camera.h"
#include "RenderTexture.h"
#include "Shader.h"
#include "Models/ObjModel.h"

#include "../ModelData.h"

Renderer r;

struct shadow_data
{
	Vec4 splits;
	Matrix4 matrices[3];
};


Renderer::Renderer()
{
	this->shadowSplits = 3;
	this->dirToLight = Vec3(0, -1, 0);
	this->shadowMapDepthBias = 0.0f;
	this->shadowMaxDist = 150;//350
	this->shadowSplitLogFactor = 0.9f;
	this->sun_light = Vec3(0.9f, 0.9, 0.9);
	this->_shadows = true;//should default to false eventually
	this->SetAmbient(Vec3(0.4, 0.4, 0.54), Vec3(0.2, 0.2, 0.2));
}

void Renderer::Init(CRenderer* renderer)
{
	shader_ss = resources.get_unsafe<CShader>("Shaders/skinned_shadow.vsh");
	shader_s = resources.get_unsafe<CShader>("Shaders/shadow.vsh");
	shader_sa = resources.get_unsafe<CShader>("Shaders/alpha_shadow.vsh");// renderer->CreateShader

	//create common constant buffers
	//bind each buffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(shadow_data);// cdesc.Size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the shader constant buffer from within this class.
	ID3D11Buffer* cbuffer;
	HRESULT result = renderer->device->CreateBuffer(&bufferDesc, NULL, &cbuffer);
	if (FAILED(result) || cbuffer == 0)
		throw 7;

	this->shadow_buffer = cbuffer;

	for (int i = 0; i < SHADOW_MAP_MAX_CASCADE_COUNT; i++)
	{
		auto drt = CRenderTexture::Create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);// DXGI_FORMAT_R24G8_TYPELESS);

		this->shadowMapViews[i] = drt->GetDepthResourceView();
		this->shadowMapTextures[i] = drt->texture_depth;
		this->shadowMapSurfaces[i] = drt;
	}

	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

	// Point filtered shadows can be faster, and may be a good choice when
	// rendering on hardware with lower feature levels. This sample has a
	// UI option to enable/disable filtering so you can see the difference
	// in quality and speed.

	auto res = renderer->device->CreateSamplerState(
		&comparisonSamplerDesc,
		&shadowSampler
		);
	if (FAILED(res))
		throw 7;

	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;


	res = renderer->device->CreateSamplerState(
		&comparisonSamplerDesc,
		&shadowSampler_linear
		);
	if (FAILED(res))
		throw 7;

	//load material textures here I guess
	for (auto ii : IMaterial::GetList())
		ii.second->Update(renderer);
}

void Renderer::Cleanup()
{
	for (int i = 0; i < SHADOW_MAP_MAX_CASCADE_COUNT; i++)
	{
		//deallocate shadow maps
		shadowMapTextures[i]->Release();
		shadowMapViews[i]->Release();
	}
}

//CSM is all about splitting the view frustum into several frusta, where each has same parameters except near and far clipping plane. For example if my camera near distance is 0.5, far distance is 100 and I have 3 cascades, subsequent cascades can have near...far distances like: 0.5 ... 7, 7 ... 25, 25 ... 100. First I calculate distance of far clipping plane for each cascade:
float Lerp2(float a, float b, float t)
{
	return a + (b - a)*t;
}

void Renderer::CalcShadowMapSplitDepths(float *outDepths, CCamera* camera, float maxdist)
{
	float camNear = max(camera->_near, 10);
	float camFar = min(min(camera->_far, shadowMaxDist), maxdist);

	//todo: have minimum first split distance
	float i_f = 1.f, cascadeCount = (float)this->shadowSplits;
	for (unsigned int i = 0; i < this->shadowSplits - 1; i++, i_f += 1.f)
	{
		outDepths[i] = Lerp2(
			camNear + (i_f / cascadeCount)*(camFar - camNear),
			camNear * powf(camFar / camNear, i_f / cascadeCount),
			shadowSplitLogFactor);
		if (i == 0 && outDepths[i] < 5/*minimum first split distance*/)
			outDepths[i] = 5;
		else if (i > 0 && outDepths[i] < outDepths[i - 1])
			throw 7;//make sure no overlap
	}
	outDepths[this->shadowSplits - 1] = camFar;
}

Matrix4 mv[SHADOW_MAP_MAX_CASCADE_COUNT * 2 + 1];
void Renderer::CalcShadowMapMatrices(
	Matrix4 &outViewProj,
	Matrix4 &outShadowMapTexXform,
	CCamera* cam, std::vector<Renderable*>* objs, int id)
{
	PROFILE("CalcShadowMapMatrices");
	Vec3 upDir = Vec3(0, 1, 0);
	if (fabsf(this->dirToLight.dot(upDir)) > 0.99f)
		upDir = Vec3(0, 0, 1);

	Vec3 axisX, axisY, axisZ = -this->dirToLight;
	axisX = axisZ.cross(upDir);
	axisX.normalize();
	axisY = axisX.cross(axisZ);

	Matrix4 view = Matrix4::BuildMatrix(axisX, axisY, axisZ);

	//need to watch out for bad AABBs
	//Now it's time for the second matrix. It is an orthogonal projection focusing on geometry of our interest. For the left to right and top to bottom boundaries I use bounds of the camera frustum, not the smallest box bounding all visible objects. Thanks to that I can avoid some popping when new objects enter viewing area. I still have to visit all objects to determine minimum and maximum Z.
	Matrix4 proj;
	if (objs->size() == 0)
		proj.MakeIdentity();
	else
	{
		float objsNear = FLT_MAX, objsFar = -FLT_MAX;
		Vec3 boxCorners[8];
		float objsXmin = FLT_MAX;
		float objsXmax = -FLT_MAX;
		float objsYmin = FLT_MAX;
		float objsYmax = -FLT_MAX;
		if (this->shadowSplits == 1)
		{
			for (auto ii : *objs)
			{
				//OBB &objBB = objs[i]->GetWorldBoundingBox();
				//objBB.GetAllCorners(boxCorners);
				//ii->aabb.m

				boxCorners[0] = view*ii->aabb.max;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
				boxCorners[1] = view*ii->aabb.min;//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
				boxCorners[2] = view*Vec3(ii->aabb.min.x, ii->aabb.max.y, ii->aabb.max.z);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
				boxCorners[3] = view*Vec3(ii->aabb.min.x, ii->aabb.min.y, ii->aabb.max.z);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
				boxCorners[4] = view*Vec3(ii->aabb.max.x, ii->aabb.min.y, ii->aabb.max.z);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
				boxCorners[5] = view*Vec3(ii->aabb.min.x, ii->aabb.max.y, ii->aabb.min.z);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z);

				//TransformArray(boxCorners, _countof(boxCorners), view);
				for (unsigned int corner_i = 0; corner_i < 6; corner_i++)
				{
					if (boxCorners[corner_i].z < objsNear) objsNear = boxCorners[corner_i].z;
					if (boxCorners[corner_i].z > objsFar) objsFar = boxCorners[corner_i].z;
					if (boxCorners[corner_i].x < objsXmin) objsXmin = boxCorners[corner_i].x;
					if (boxCorners[corner_i].x > objsXmax) objsXmax = boxCorners[corner_i].x;
					if (boxCorners[corner_i].y < objsYmin) objsYmin = boxCorners[corner_i].y;
					if (boxCorners[corner_i].y > objsYmax) objsYmax = boxCorners[corner_i].y;
				}
			}
		}
		else
		{
			float yfar = cam->_far*tan(cam->_fov / 2.0f);
			float xfar = yfar*cam->_aspectRatio;
			float ztotal = cam->_far - cam->_near;
			float ynear = cam->_near*tan(cam->_fov / 2.0f);
			float xnear = ynear*cam->_aspectRatio;

			Vec3 _right = cam->GetRight();
			Vec3 _lookAt = cam->GetForward();
			Vec3 _upDir = cam->GetUp();
			boxCorners[0] = view*(cam->_pos + _right*xnear + _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
			boxCorners[1] = view*(cam->_pos + _right*xnear - _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
			boxCorners[2] = view*(cam->_pos + _right*xfar + _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[3] = view*(cam->_pos - _right*xnear - _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
			boxCorners[4] = view*(cam->_pos - _right*xnear + _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
			boxCorners[5] = view*(cam->_pos + _right*xfar - _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[6] = view*(cam->_pos - _right*xfar + _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[7] = view*(cam->_pos - _right*xfar - _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);

			for (unsigned int corner_i = 0; corner_i < 8; corner_i++)
			{
				//if (boxCorners[corner_i].z < objsNear) objsNear = boxCorners[corner_i].z;
				if (boxCorners[corner_i].z > objsFar) objsFar = boxCorners[corner_i].z;
				if (boxCorners[corner_i].x < objsXmin) objsXmin = boxCorners[corner_i].x;
				if (boxCorners[corner_i].x > objsXmax) objsXmax = boxCorners[corner_i].x;
				if (boxCorners[corner_i].y < objsYmin) objsYmin = boxCorners[corner_i].y;
				if (boxCorners[corner_i].y > objsYmax) objsYmax = boxCorners[corner_i].y;
			}

			AABB objbounds(FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
			for (auto ii : *objs)
			{
#ifdef _DEBUG
				if (ii->aabb.max.x == FLT_MAX)
					throw "Invalid Bounding Box!";
#endif
				objbounds.FitAABB(ii->aabb);
			}
			//renderer->DrawBoundingBox(objbounds);
			objbounds.Transform(view);

			//can start constraining x and y based on objects, as well as the frustum, for space and what not
			objsNear = objbounds.min.z;
			//take smallest value to use as max, frustum or objects (no need to cast shadows from things behind us relative to the light)
			objsFar = min(objbounds.max.z, objsFar);
		}

		//fix this stuff
		if (true)//makes shadows stay put while moving showdebug > 0)
		{
			float qStepX = (objsXmax - objsXmin/*1.0f*/) / SHADOW_MAP_SIZE;
			float qStepY = (objsYmax - objsYmin/*1.0f*/) / SHADOW_MAP_SIZE;

			objsXmin /= qStepX;
			objsXmin = floor(objsXmin);
			objsXmin *= qStepX;

			objsXmax /= qStepX;
			objsXmax = floor(objsXmax);
			objsXmax *= qStepX;

			objsYmin /= qStepY;
			objsYmin = floor(objsYmin);
			objsYmin *= qStepY;

			objsYmax /= qStepY;
			objsYmax = floor(objsYmax);
			objsYmax *= qStepY;
		}

		if (objsNear == objsFar)
			proj.MakeIdentity();
		else
		{
			//clamp this to points
			proj = Matrix4::OrthographicOffCenterLHMatrix(
				objsXmin,// left
				objsXmax,// right
				objsYmin,// bottom
				objsYmax,// top
				objsNear,// near
				objsFar);// far
		}
	}
	outViewProj = proj * view.Transpose();

	if (showdebug == 0)
	{
		mv[id * 2] = view;
		mv[id * 2 + 1] = proj;
	}

	//The outViewProj matrix will be used for rendering objects to shadow map as a render target. Now calculation of the second matrix remain. It is similar to outViewProj, but it will be used to sample shadow map as a texture. The only difference is that we have to transform vertices to texture coordinates where x and y are 0...1, not -1...1 as it was in rendering to shadow map. I also apply depth bias here to avoid self-shadowing and minimize z-acne effect. Doing both is just the matter of applying additional, simple correction transformation to the matrix I calculated before:
	Matrix4 viewportToTex = Matrix4(
		0.5f, 0.f, 0.f, 0.f,
		0.f, -0.5f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.5f, 0.5f, shadowMapDepthBias, 0.f);
	outShadowMapTexXform = viewportToTex.Transpose() * outViewProj;
}


void Renderer::RenderShadowMap(int id, std::vector<Renderable*>* objs, const Matrix4& viewProj)
{
	renderer->EnableAlphaBlending(false);

	CRenderTexture oldrt = renderer->GetRenderTarget(0);

	renderer->context->ClearDepthStencilView(this->shadowMapSurfaces[id]->depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	renderer->context->OMSetRenderTargets(1, &this->shadowMapSurfaces[id]->color, this->shadowMapSurfaces[id]->depth);

	renderer->SetCullmode(CULL_CCW);

	CShader* atest = this->shader_sa;

	//unbind PS

	//lets do a little hack since we dont need the pixel shaders
	this->shader_s->pshader = 0;
	this->shader_ss->pshader = 0;
	//ok, this is difficult to do, need to extract actual renderering from submitting the commands
	CShader* shader;
	Matrix4 shadowmat = viewProj.Transpose();
	for (auto obj : *objs)
	{
		if (obj->type == Skinned)
			shader = renderer->SetShader(this->shader_ss);// 13);
		else
			shader = renderer->SetShader(this->shader_s);//2);

		Matrix4 world = (obj->matrix*shadowmat).Transpose();

		//renderer->context->PSSetShader(0, 0, 0);

		if (obj->material)
		{
			if (obj->material->alphatest == false)
			{
				//renderer->SetPixelTexture(0, 0);//commented out because probably not necessary
				//renderer->context->PSSetShader(0, 0, 0);
			}
			else
			{
				renderer->SetShader(atest);

				//need to set vertex shader too...
				renderer->SetPixelTexture(0, obj->material->texture);
				renderer->SetFilter(0, FilterMode::Linear);
			}

			if (obj->material->cullmode == CULL_NONE)
				renderer->SetCullmode(CULL_NONE);
			else
				renderer->SetCullmode(CULL_CCW);
		}


		auto mat = shader->buffers.wvp;
		if (mat.buffer)
		{
			D3D11_MAPPED_SUBRESOURCE cb;
			renderer->context->Map(mat.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
			struct mdata
			{
				Matrix4 wvp;
			};
			auto data = (mdata*)cb.pData;
			data->wvp = world;
			renderer->context->Unmap(mat.buffer, 0);

			renderer->context->VSSetConstantBuffers(mat.vsslot, 1, &mat.buffer);
		}

		switch (obj->type)
		{
		case Standard:
		{
			obj->vb->Bind();

			int vcount = obj->vb->GetSize() / obj->vb->GetStride();

			renderer->DrawPrimitive(PT_TRIANGLELIST, 0, vcount);

			break;
		}
		case Skinned:
		{
			ObjModel* model = static_cast<ObjModel*>(obj);

			if (model->animate)
			{
				auto mat = shader->buffers.skinning;
				if (mat.buffer)
				{
					D3D11_MAPPED_SUBRESOURCE cb;
					int bones = model->data->num_joints;
					renderer->context->Map(mat.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
					struct mdata2
					{
						Matrix3x4 mats[70];
					};
					auto data = (mdata2*)cb.pData;
					for (int i = 0; i < bones; i++)
						data->mats[i] = model->OutFrames[i];
					renderer->context->Unmap(mat.buffer, 0);

					renderer->context->VSSetConstantBuffers(mat.vsslot, 1, &mat.buffer);
				}
			}
		}
		case Indexed:
		{
			ObjModel* model = static_cast<ObjModel*>(obj);
			model->data->vb.Bind();
			for (int i = 0; i < model->data->num_meshes; i++)
			{
				Mesh* mesh = &model->data->meshes[i];
				mesh->ib->Bind();

				if (mesh->material->alphatest == false)
				{
					//renderer->SetPixelTexture(0, 0);//commented out because probably not necessary
					renderer->SetShader(shader);

					//renderer->context->PSSetShader(0, 0, 0);
					//this could cause an issue with the wrong IL being bound since renderer->shader is still the same
				}
				else
				{
					renderer->SetShader(atest);

					//need to set vertex shader too...
					renderer->SetPixelTexture(0, mesh->material->texture);
					renderer->SetFilter(0, FilterMode::Linear);
				}

				if (mesh->material->cullmode == CULL_NONE)
					renderer->SetCullmode(CULL_NONE);
				else
					renderer->SetCullmode(CULL_CCW);

				renderer->DrawIndexedPrimitive(PT_TRIANGLELIST, 0, 0, mesh->num_vertexes, mesh->num_triangles * 3);
			}
			break;
		}
		}
	}

	//set rt back
	//renderer->SetRenderTarget(0, &oldrt);
	renderer->context->OMSetRenderTargets(1, &oldrt.color, oldrt.depth);

	//renderer->context->OMSetRenderTargets(1, &renderer->renderTargetView, renderer->depthStencilView);

	renderer->SetCullmode(CULL_CW);
}

//finish material system and get automatic lighting
void BuildShadowFrustum(CCamera* cam, CCamera& out, Vec3 _DirToLight)
{
	//build view matrix
	Vec3 upDir = Vec3(0, 1, 0);
	if (fabsf(_DirToLight.dot(upDir)) > 0.99f)
		upDir = Vec3(0, 0, 1);

	Vec3 axisX, axisY, axisZ = -_DirToLight;
	axisX = axisZ.cross(upDir);
	axisX.normalize();
	axisY = axisX.cross(axisZ);

	Matrix4 view = Matrix4::BuildMatrix(axisX, axisY, axisZ);
	out._matrix = view;

	//fit frustum corners
	Vec3 min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3 max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	float yfar = cam->_far*tan(cam->_fov / 2.0f);
	float xfar = yfar*cam->_aspectRatio;
	float ztotal = cam->_far - cam->_near;
	float ynear = cam->_near*tan(cam->_fov / 2.0f);
	float xnear = ynear*cam->_aspectRatio;

	Vec3 _right = cam->GetRight();
	Vec3 _upDir = cam->GetUp();
	Vec3 _lookAt = cam->GetForward();

	Vec3 boxCorners[8];
	boxCorners[0] = view*(cam->_pos + _right*xnear + _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
	boxCorners[1] = view*(cam->_pos + _right*xnear - _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
	boxCorners[2] = view*(cam->_pos + _right*xfar + _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[3] = view*(cam->_pos - _right*xnear - _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
	boxCorners[4] = view*(cam->_pos - _right*xnear + _upDir*ynear + _lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
	boxCorners[5] = view*(cam->_pos + _right*xfar - _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[6] = view*(cam->_pos - _right*xfar + _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[7] = view*(cam->_pos - _right*xfar - _upDir*yfar + _lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);

	for (unsigned int corner_i = 0; corner_i < 8; corner_i++)
	{
		if (boxCorners[corner_i].z < min.z) min.z = boxCorners[corner_i].z;
		if (boxCorners[corner_i].z > max.z) max.z = boxCorners[corner_i].z;
		if (boxCorners[corner_i].x < min.x) min.x = boxCorners[corner_i].x;
		if (boxCorners[corner_i].x > max.x) max.x = boxCorners[corner_i].x;
		if (boxCorners[corner_i].y < min.y) min.y = boxCorners[corner_i].y;
		if (boxCorners[corner_i].y > max.y) max.y = boxCorners[corner_i].y;
	}

	out.OrthoProjection(min.x, max.x, min.y, max.y, min.z, max.z);
	out.BuildViewFrustum();
}


void Renderer::Render(CCamera* cam, CRenderer* render)//if the renderable's parent is the same as the cameras, just render nicely
{
	RPROFILE("RendererProcess");
	GPUPROFILEGROUP("Renderer::Render");

	//execute pre-frame commands (from other threads and such
	todo_lock.lock();//lock
	while (todo.size())
	{
		todo.back()();
		todo.pop();
	}
	todo_lock.unlock();//unlock


	renderer->ApplyCam(cam);

	this->rcount = this->renderables.size();

	//camera contains local camera angles
	Parent* parent = cam->parent;
	this->cur_parent = cam->parent;

	//lets generalize some more and get multiple parent hierachys working

	//compute both local and global camera matrices
	Matrix4 localview = cam->_matrix;
	Matrix4 globalview;
	if (parent)
	{
		globalview = parent->mat*localview;//todo, handle stacks of parents
		Parent* current = parent->parent;
		while (false)//current)
		{
			globalview *= parent->mat;
			current = parent->parent;
		}
	}
	else
		globalview = localview;

	this->rdrawn = 0;

	//ok, heres order we have to do for normal rendering
	//1. cull renderables to frustum
	//2. have each renderable go through and make submissions
	//2. calculate sort indices 
	//3. sort submissions by distance and shader
	//4. render

	//each renderable needs to submit subcommands with materials
	std::vector<RenderCommand> renderqueue;
	renderqueue.reserve(300);//lets try and not recycle this

	Vec3 worldcampos = globalview.GetTranslation();

	this->current_matrix = 0;

	/* 1. CULL AND SUBMIT STAGE*/
	for (unsigned int i = 0; i < this->renderables.size(); i++)//loop through all renderables to calculate sorting factors
	{
		Renderable* r = this->renderables[i];
		if (r->parent != cam->parent || r->aabb.max.x == FLT_MAX || cam->BoxInFrustum(r->aabb))//frustum cull
		{
			/* 2. Calculate sort indices! */
			Vec3 pos = r->matrix.GetTranslation();
			if (r->parent == cam->parent)
				r->dist = cam->_pos.distsqr(pos);
			else if (r->parent == 0)
				r->dist = worldcampos.distsqr(pos);
			else//get to global
			{
				Vec3 wpos = r->parent->LocalToWorld(r->matrix.GetTranslation());
				r->dist = worldcampos.distsqr(wpos);
			}

			//push children renderables to list and process
			int old_size = renderqueue.size();
			r->Render(cam, &renderqueue);

			//update matrices
			/*for (int i = old_size; i < renderqueue.size(); i++)
			{
			auto ptr = &this->matrix_block[this->current_matrix++];
			renderqueue[i].transform = ptr;
			*ptr = renderqueue[i].source->matrix;
			}*/

			//update predraw hooks
			if (r->updated == false)
			{
				//if we have an entity, update it before rendering
				if (r->entity)
					r->entity->PreRender();
				r->updated = true;
			}
		}
	}
	//add more shadow settings

	//should be in a flat tree like layout, parents always need to come before children
	//topological sort ftw
	//all transform heirachys so add dirty flags
	//ok, to draw shadows, we need to make sure we are in right area, then apply world*shadowvp as the world*view*projection matrix
	Matrix4 shadowMapViewProjs[SHADOW_MAP_MAX_CASCADE_COUNT];

	//clear resources
	ID3D11ShaderResourceView* stuff[4] = { 0 };
	renderer->context->PSSetShaderResources(1, shadowSplits, stuff);

	//could try and only draw this every other frame on low end computers
	this->RenderShadowMaps(shadowMapViewProjs, cam);

	/* 3. Sort by distance and shader *///need to also sort by shader/texture, especially for shadows
	{
		PROFILE("SortRenderables");
		std::sort(renderqueue.begin(), renderqueue.end(),
			[](const RenderCommand& a, const RenderCommand& b)
		{
			if (a.alpha && b.alpha)
				return a.dist > b.dist;
			else if (a.alpha)
				return false;
			else if (b.alpha)
				return true;

			return a.dist < b.dist;
		});
	}

	//setup vars for shadows
	if (this->_shadows)
	{
		for (int li = 0; li < shadowSplits; li++)
			renderer->SetPixelTexture(li + 1, this->shadowMapViews[li]);

		//setup shadow filters
		renderer->context->PSSetSamplers(3, 1, &this->shadowSampler_linear);
		//renderer->context->PSSetSamplers(3, 1, &this->shadowSampler);
	}

	//sort the lights by size so that largest lights get picked
	std::sort(this->lights.begin(), this->lights.end(), [](const Light& a, const Light& b)
	{
		return a.radius > b.radius;
	});

	//ok, lets be dumb and update materials here
	for (auto ii : IMaterial::GetList())
		ii.second->Update(renderer);

	//update common constant buffers
	{
		//update shadow constant buffer
		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(this->shadow_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		auto data = (shadow_data*)cb.pData;
		data->splits = Vec4(this->shadowMappingSplitDepths);
		data->matrices[0] = shadowMapTexXforms[0];
		data->matrices[1] = shadowMapTexXforms[1];
		data->matrices[2] = shadowMapTexXforms[2];
		renderer->context->Unmap(this->shadow_buffer, 0);
	}

	/* 4. RENDER OBJECTS */
	GPUPROFILE2("Execute Commands");
	//todo why am I doing this
	renderer->SetFilter(10, FilterMode::Point);

	render->SetMatrix(VIEW_MATRIX, &globalview);

	//execute all of the render commands
	this->ProcessQueue(cam, renderqueue);

	//removing old lights
	for (int i = 0; i < this->lights.size(); i++)
	{
		if (this->lights[i].lifetime > 0)
		{
			this->lights[i].lifetime -= 0.01;//hack
			if (this->lights[i].lifetime < 0)
				this->lights.erase(this->lights.begin() + i--);
		}
	}

	//debug draw shadow frustums
	if (showdebug == 0)
		mv[SHADOW_MAP_MAX_CASCADE_COUNT * 2] = cam->_matrix;
	if (showdebug > 1)
	{
		renderer->ApplyCam(cam);

		for (int cascade_i = 0; cascade_i < shadowSplits; cascade_i++)
		{
			COLOR colors[] = { 0x14FF0000, 0x1400FF00, 0x140000FF };
			renderer->DrawFrustum(mv[cascade_i * 2], mv[cascade_i * 2 + 1], colors[cascade_i]);
		}
		for (int i = shadowSplits - 1; i >= 0; i--)
		{
			CCamera tmpCamera = *cam;
			tmpCamera._far = shadowMappingSplitDepths[i];
			if (i > 0)
				tmpCamera._near = shadowMappingSplitDepths[i - 1];
			tmpCamera.PerspectiveProjection();

			COLOR colors[] = { 0x14FF0000, 0x1400FF00, 0x140000FF };

			renderer->DrawFrustum(mv[6], tmpCamera._projectionMatrix, colors[i]);
		}
	}
}


void Renderer::SetupMaterials(const RenderCommand* rc)
{

}

void Renderer::UpdateUniforms(const RenderCommand* rc, const CShader* shader, const Matrix4* shadowMapTexXforms, bool shader_changed, const Light* light_list)
{
	//clean this up ok
	//todo, only do this if the rc source has changed
	auto matrix = &rc->source->matrix;// rc->transform;
	if (shader->buffers.matrices.buffer)
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(shader->buffers.matrices.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		struct mdata
		{
			Matrix4 world;
			Matrix4 view;
			Matrix4 projection;
			Matrix4 wvp;
		};
		auto data = (mdata*)cb.pData;
		data->world = matrix->Transpose();
		data->view = renderer->view.Transpose();
		data->projection = renderer->projection.Transpose();
		auto wVP = (*matrix)*renderer->view*renderer->projection;
		wVP.MakeTranspose();
		data->wvp = wVP;

		renderer->context->Unmap(shader->buffers.matrices.buffer, 0);

		if (shader_changed)
			renderer->context->VSSetConstantBuffers(shader->buffers.matrices.vsslot, 1, &shader->buffers.matrices.buffer);
	}

	//simple buffer
	if (shader->buffers.wvp.buffer)
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(shader->buffers.wvp.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		struct mdata
		{
			Matrix4 wvp;
		};
		auto data = (mdata*)cb.pData;
		auto wVP = (*matrix)*renderer->view*renderer->projection;
		wVP.MakeTranspose();
		data->wvp = wVP;
		renderer->context->Unmap(shader->buffers.wvp.buffer, 0);

		if (shader_changed)
			renderer->context->VSSetConstantBuffers(shader->buffers.wvp.vsslot, 1, &shader->buffers.wvp.buffer);
	}

	//ok, lets move this out and stop updating it so much
	if (shader->buffers.shadow.buffer && shader_changed)
	{
		//for (int i = 0; i < shadowSplits; i++)
		//	renderer->SetPixelTexture(i + 1, this->shadowMapViews[i]);

		renderer->context->PSSetConstantBuffers(shader->buffers.shadow.psslot, 1, &this->shadow_buffer);// shader->buffers.shadow.buffer);
	}

	if (shader->buffers.lighting.buffer)
	{
		Vec3 dir = this->dirToLight;
		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(shader->buffers.lighting.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
#pragma pack(push)
#pragma pack(16)
		struct pl
		{
			Vec4 pos;
			Vec4 color;
		};
		struct mdata
		{
			Vec4 direction;
			Vec4 daylight;
			Vec4 ambient_down;
			Vec4 ambient_range;
			pl lights[3];
		};
#pragma pack(pop)
		auto data = (mdata*)cb.pData;
		data->direction.xyz = dir;
		data->ambient_down.xyz = this->ambient_bottom;
		data->ambient_range.xyz = this->ambient_range;
		//data->ambient.xyz = this->ambient;// Vec4(0.2175f, 0.2175, 0.2175, 0.2175);//rc->source->ambientlight;
		data->daylight = Vec4(this->sun_light, 1);//rc->source->daylight;
		for (int i = 0; i < 3; i++)
		{
			data->lights[i].pos.xyz = light_list[i].position;
			data->lights[i].color.xyz = light_list[i].color;
			data->lights[i].pos.w = light_list[i].radius;
		}
		renderer->context->Unmap(shader->buffers.lighting.buffer, 0);

		if (shader_changed)
		{
			if (shader->buffers.lighting.psslot >= 0)
				renderer->context->PSSetConstantBuffers(shader->buffers.lighting.psslot, 1, &shader->buffers.lighting.buffer);
			if (shader->buffers.lighting.vsslot >= 0)
				renderer->context->VSSetConstantBuffers(shader->buffers.lighting.vsslot, 1, &shader->buffers.lighting.buffer);
		}
	}

	if (shader->buffers.skinning.buffer && rc->mesh.OutFrames)
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		int bones = static_cast<ObjModel*>(rc->source)->data->num_joints;
		renderer->context->Map(shader->buffers.skinning.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		struct mdata2
		{
			Matrix3x4 mats[70];
		};
		auto data = (mdata2*)cb.pData;
		for (int i = 0; i < bones; i++)
			data->mats[i] = rc->mesh.OutFrames[i];
		renderer->context->Unmap(shader->buffers.skinning.buffer, 0);

		if (shader_changed)
			renderer->context->VSSetConstantBuffers(shader->buffers.skinning.vsslot, 1, &shader->buffers.skinning.buffer);
	}
	//perhaps this should be done per renderable, not per command
}

void Renderer::Render(CCamera* cam, Renderable* r)
{
	/* 4. RENDER OBJECTS */
	//Parent* last = 0;//keeps track of what view matix is active
	//IMaterial* lastm = (IMaterial*)-1;
	//todo: only do this once per frame
	//ok, lets be dumb and update materials here
	for (auto ii : IMaterial::GetList())
		ii.second->Update(renderer);

	std::vector<RenderCommand> renderqueue;

	r->Render(cam, &renderqueue);

	renderer->ApplyCam(cam);

	this->ProcessQueue(cam, renderqueue);
}

void Renderer::ProcessQueue(CCamera* cam, const std::vector<RenderCommand>& renderqueue)
{
	//compute both local and global camera matrices
	Parent* parent = this->cur_parent;
	Matrix4 localview = cam->_matrix;
	Matrix4 globalview;
	if (parent)
	{
		globalview = parent->mat*localview;//todo, handle stacks of parents
		Parent* current = parent->parent;
		while (false)//current)
		{
			globalview *= parent->mat;
			current = parent->parent;
		}
	}
	else
		globalview = localview;

	Parent* last = 0;//keeps track of what view matix is active
	IMaterial* lastm = (IMaterial*)-1;
	for (int i = 0; i < renderqueue.size(); i++)
	{
		const RenderCommand& rc = renderqueue[i];
		//lets fill this with all the data we need, so we dont need
		//to go back to the source

		if (last != rc.source->parent)//do I need to change the current view matrix?
		{
			if (rc.source->parent == parent)//object has same parent as the camera
			{
				renderer->SetMatrix(VIEW_MATRIX, &localview);
			}
			else if (rc.source->parent)//object has different parent
			{
				//need to recurse down to get correct matrix
				//goes from left to right so
				//temp = (matrix1*matrix2)*matrix3)*matrix4
				Matrix4 temp = rc.source->parent->mat.Inverse()*globalview;
				renderer->SetMatrix(VIEW_MATRIX, &temp);
			}
			else//object is in global space and has no parent
			{
				renderer->SetMatrix(VIEW_MATRIX, &globalview);
			}
			last = rc.source->parent;
		}

		//apply material settings
		bool shaderchange = false;
		if (lastm != rc.material)
		{
			rc.material->Apply(renderer);//make apply not apply the shader
			lastm = rc.material;
			shaderchange = true;
		}

		//find applicable lights then set'em up
		int num_lights = 0;
		Light found_lights[6];//max per pixel
		//move this into an external function to clean up this code
		//hack for the moment 
		const Vec3 position = rc.position;
		const float radius = rc.radius;
		//todo: get enemy spawning system and deaths
		//todo: perhaps allow more lights if necessary
		for (auto light : this->lights)
		{
			if (num_lights < 6 && light.position.dist(position) < (light.radius + radius)/*entity radius needs to go here*/)
			{
				//apply it
				found_lights[num_lights++] = light;
			}
		}


		auto oldshdr = renderer->shader;

		//ok, need to select right shader for skinned / nonskinned
		//todo: also do shader LOD here
		rc.material->ApplyShader(rc.mesh.OutFrames, num_lights);
		shaderchange = (oldshdr != renderer->shader);

		//apply per instance values
		//use me for color and stuffs
		//only need to do this for shader builder shaders
		//todo, use materials that have/dont have these so we dont do extra work
		//todo make this one command
		if (rc.material_instance.extra)
			renderer->SetPixelTexture(8, rc.material_instance.extra);
		if (rc.material_instance.extra2)
			renderer->SetPixelTexture(9, rc.material_instance.extra2);
		if (renderer->shader->cbuffers.find("color") != renderer->shader->cbuffers.end())
		{
			unsigned int col = rc.material_instance.color;
			if (col == 0)
				col = 0xFFFFFFFF;
			Vec4 color;//argb
			color[0] = (col >> 16) & 0xFF;
			color[1] = (col >> 8) & 0xFF;
			color[2] = (col)& 0xFF;
			color[3] = 255;
			color /= 255;

			//order needs to be rgba
			renderer->shader->cbuffers["color"].UploadAndSet(&color, 4 * 4);
		}
		else
		{
			//ID buffer = 0;
			//renderer->context->PSSetConstantBuffers(2, 1, 0);
		}

		/* execute render command */
		CShader* shader = renderer->shader;
		this->UpdateUniforms(&rc, shader, shadowMapTexXforms, shaderchange, found_lights);

		rc.mesh.vb->Bind();//maybe make this more data oriented?

		//todo: improve the speed of these
		//		shouldnt always be setting input layout
		//	    and primitive topology
		if (rc.mesh.ib == 0)
			renderer->DrawPrimitive(PT_TRIANGLELIST, 0, rc.mesh.primitives);
		else
		{
			rc.mesh.ib->Bind();//make this more data oriented, can definately remove this indirection

			renderer->DrawIndexedPrimitive(PT_TRIANGLELIST, 0, 0, rc.mesh.primitives, rc.mesh.num_indices);
		}

		this->rdrawn++;
	}
}

void Renderer::RenderShadowMaps(Matrix4* shadowMapViewProjs, CCamera* cam)
{
	if (this->_shadows == false)
		return;

	PROFILE("DrawShadows");
	GPUPROFILE("RenderShadows");

	//setup viewport
	Viewport vp;
	renderer->GetViewport(&vp);
	int ow = vp.Width;
	int oh = vp.Height;
	int ox = vp.X;
	vp.X = 0;
	vp.Width = SHADOW_MAP_SIZE;
	vp.Height = SHADOW_MAP_SIZE;
	renderer->SetViewport(&vp);

	//draw to shadow map
	CalcShadowMapSplitDepths(shadowMappingSplitDepths, cam, 200);
	for (unsigned int cascade_i = 0; cascade_i < this->shadowSplits; cascade_i++)
	{
		//need to adjust shadow maps to fit around objects
		CCamera tmpCamera = *cam;
		tmpCamera.SetFar(shadowMappingSplitDepths[cascade_i]);
		if (cascade_i > 0)
			tmpCamera.SetNear(shadowMappingSplitDepths[cascade_i - 1]);

		//tmpCamera.doProjection();
		tmpCamera.DoMatrix();

		//build basic frustum that fits the tmpCamera
		CCamera culling;
		BuildShadowFrustum(&tmpCamera, culling, this->dirToLight);

		//need to make list of all renderables in frustum
		std::vector<Renderable*> locals;
		for (auto ren : this->renderables)
		{
			//submit to render queue
			if (ren->castsShadows && ren->parent == cam->parent)
			{
				//need to cull against light dir
				if (culling.BoxInFrustumSidesAndFar(ren->aabb))
				{
					if (ren->updated == false)
					{
						//if we have an entity, update it before rendering
						if (ren->entity)
							ren->entity->PreRender();
						ren->updated = true;
					}
					locals.push_back(ren);
				}
			}
		}

		//refine matrix to fit all meshes tightly
		CalcShadowMapMatrices(
			shadowMapViewProjs[cascade_i],
			shadowMapTexXforms[cascade_i],
			&tmpCamera, &locals, cascade_i);

		//draw each
		RenderShadowMap(cascade_i, &locals, shadowMapViewProjs[cascade_i]);
	}

	//renderer->context->OMSetRenderTargets(1, &renderer->renderTargetView, renderer->depthStencilView);

	vp.X = ox;
	vp.Width = ow;
	vp.Height = oh;
	renderer->SetViewport(&vp);
	if (showdebug > 1)
	{
		//draw shadow maps
		renderer->SetPixelTexture(0, this->shadowMapViews[0]);
		Rect r(300, 600, 0, 300);
		renderer->DrawRect(&r, 0xFFFFFFF);

		if (this->shadowSplits > 1)
		{
			renderer->SetPixelTexture(0, this->shadowMapViews[1]);
			r = Rect(300, 600, 300, 600);
			renderer->DrawRect(&r, 0xFFFFFFF);
		}
		if (this->shadowSplits > 2)
		{
			renderer->SetPixelTexture(0, this->shadowMapViews[2]);
			r = Rect(0, 300, 0, 300);
			renderer->DrawRect(&r, 0xFFFFFFF);
		}
		if (this->shadowSplits > 3)
		{
			renderer->SetPixelTexture(0, this->shadowMapViews[3]);
			r = Rect(0, 300, 300, 600);
			renderer->DrawRect(&r, 0xFFFFFFF);
		}
	}
}