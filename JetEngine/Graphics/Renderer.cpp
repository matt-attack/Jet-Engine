#include "Renderer.h"
#include "CRenderer.h"
#include "../Util/Profile.h"

#include "../IMaterial.h"
#include <D3DX11.h>

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
	this->_shadows = true;//should default to false eventually
}

void Renderer::Init(CRenderer* renderer)
{
	//todo: stop using renderer list of shaders
	shader_ss = renderer->CreateShader(13, "Shaders/skinned_shadow.vsh");

	shader_s = renderer->CreateShader(2, "Shaders/shadow.vsh");
	//allocate depth buffer
	/*D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory((void*)&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = SHADOW_MAP_SIZE;
	depthBufferDesc.Height = SHADOW_MAP_SIZE;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	ID3D11Texture2D* depthStencilBuffer;
	HRESULT result = renderer->device->CreateTexture2D(&depthBufferDesc, 0, &depthStencilBuffer);
	if (FAILED(result))
	throw 7;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = depthBufferDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = renderer->device->CreateDepthStencilView(depthStencilBuffer,&depthStencilViewDesc, &depthStencilView);
	if (FAILED(result))
	throw 7;*/

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
		//allocate shadow maps
		D3D11_TEXTURE2D_DESC desc;
		memset(&desc, 0, sizeof(D3D11_TEXTURE2D_DESC));

		// Setup the render target texture description.
		desc.Width = SHADOW_MAP_SIZE;
		desc.Height = SHADOW_MAP_SIZE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		ID3D11Texture2D* renderTargetTexture;
		// Create the render target texture.
		/*auto result = renderer->device->CreateTexture2D(&desc, NULL, &renderTargetTexture);
		if(FAILED(result))
		{
		throw 7;
		}*/

		//this should be MUCH faster
		D3D11_TEXTURE2D_DESC shadowMapDesc;
		ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		shadowMapDesc.MipLevels = 1;
		shadowMapDesc.ArraySize = 1;
		shadowMapDesc.SampleDesc.Count = 1;
		shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		shadowMapDesc.Height = static_cast<UINT>(1024);
		shadowMapDesc.Width = static_cast<UINT>(1024);

		HRESULT hr = renderer->device->CreateTexture2D(
			&shadowMapDesc,
			nullptr,
			&renderTargetTexture
			);
		if (FAILED(hr))
			throw 7;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		ID3D11DepthStencilView* shadowDepthView;
		hr = renderer->device->CreateDepthStencilView(
			renderTargetTexture,
			&depthStencilViewDesc,
			&shadowDepthView
			);
		if (FAILED(hr))
			throw 7;

		ID3D11RenderTargetView* renderTargetView;

		hr = renderer->device->CreateShaderResourceView(
			renderTargetTexture,
			&shaderResourceViewDesc,
			&this->shadowMapViews[i]
			);
		if (FAILED(hr))
			throw 7;


		this->shadowMapTextures[i] = renderTargetTexture;
		this->shadowMapSurfaces[i] = new CRenderTexture;
		this->shadowMapSurfaces[i]->color = 0;//renderTargetView;//(renderTargetView,0);
		this->shadowMapSurfaces[i]->depth = shadowDepthView;//0;
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
	{
		ii.second->Update(renderer);
	}
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

			boxCorners[0] = view*(cam->_pos + cam->_right*xnear + cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
			boxCorners[1] = view*(cam->_pos + cam->_right*xnear - cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
			boxCorners[2] = view*(cam->_pos + cam->_right*xfar + cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[3] = view*(cam->_pos - cam->_right*xnear - cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
			boxCorners[4] = view*(cam->_pos - cam->_right*xnear + cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
			boxCorners[5] = view*(cam->_pos + cam->_right*xfar - cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[6] = view*(cam->_pos - cam->_right*xfar + cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
			boxCorners[7] = view*(cam->_pos - cam->_right*xfar - cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);

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

#include "RenderTexture.h"
void Renderer::RenderShadowMap(int id, std::vector<Renderable*>* objs, const Matrix4& viewProj)
{
	renderer->EnableAlphaBlending(false);

	CRenderTexture oldrt = renderer->GetRenderTarget(0);

	//IDirect3DSurface9* surf;
	//this->ShadowMapTextures[id]->GetSurfacelevel(0, &surf);
	//renderer->SetRenderTarget(0, this->ShadowMapSurfaces[id]);
	//renderer->context->OMSetRenderTargets(1, &this->ShadowMapSurfaces[id]->color, depthStencilView);
	//pDev->SetRenderTarget(0,surf);
	//pDev->SetDepthStencilSurface(this->ShadowMapSurfaces[id]);

	//this->shadowMapSurfaces[id]->Clear(1,0,1,1);
	renderer->context->ClearDepthStencilView(this->shadowMapSurfaces[id]->depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	renderer->context->OMSetRenderTargets(1, &this->shadowMapSurfaces[id]->color, this->shadowMapSurfaces[id]->depth);

	renderer->SetCullmode(CULL_CCW);

	//unbind PS

	CShader* shader;
	Matrix4 shadowmat = viewProj.Transpose();
	for (auto obj : *objs)
	{
		if (obj->type == Skinned)
			shader = renderer->SetShader(this->shader_ss);// 13);
		else
			shader = renderer->SetShader(this->shader_s);//2);

		Matrix4 world = (obj->matrix*shadowmat).Transpose();

		renderer->context->PSSetShader(0, 0, 0);

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
					int bones = model->t->num_joints;
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
			model->t->vb.Bind();
			for (int i = 0; i < model->t->num_meshes; i++)
			{
				Mesh* mesh = &model->t->meshes[i];
				mesh->ib->Bind();

				renderer->DrawIndexedPrimitive(PT_TRIANGLELIST, 0, 0, mesh->num_vertexes, mesh->num_triangles * 3);

				//renderer->DrawIndexedPrimitive(PT_TRIANGLELIST,0,mesh->first_vertex,mesh->num_vertexes,0,mesh->num_triangles);
			}
			break;
		}
		}
	}

	//set rt back
	//renderer->SetRenderTarget(0, &oldrt);
	renderer->context->OMSetRenderTargets(1, &renderer->renderTargetView, renderer->depthStencilView);

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

	Vec3 boxCorners[8];
	boxCorners[0] = view*(cam->_pos + cam->_right*xnear + cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
	boxCorners[1] = view*(cam->_pos + cam->_right*xnear - cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
	boxCorners[2] = view*(cam->_pos + cam->_right*xfar + cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[3] = view*(cam->_pos - cam->_right*xnear - cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
	boxCorners[4] = view*(cam->_pos - cam->_right*xnear + cam->_upDir*ynear + cam->_lookAt*cam->_near);//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
	boxCorners[5] = view*(cam->_pos + cam->_right*xfar - cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[6] = view*(cam->_pos - cam->_right*xfar + cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	boxCorners[7] = view*(cam->_pos - cam->_right*xfar - cam->_upDir*yfar + cam->_lookAt*cam->_far);//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);

	for (unsigned int corner_i = 0; corner_i < 8; corner_i++)
	{
		if (boxCorners[corner_i].z < min.z) min.z = boxCorners[corner_i].z;
		if (boxCorners[corner_i].z > max.z) max.z = boxCorners[corner_i].z;
		if (boxCorners[corner_i].x < min.x) min.x = boxCorners[corner_i].x;
		if (boxCorners[corner_i].x > max.x) max.x = boxCorners[corner_i].x;
		if (boxCorners[corner_i].y < min.y) min.y = boxCorners[corner_i].y;
		if (boxCorners[corner_i].y > max.y) max.y = boxCorners[corner_i].y;
	}

	out.orthoProjection(min.x, max.x, min.y, max.y, min.z, max.z);
	out.BuildViewFrustum();
}

//speed up chunk generation, or move loading other planets to a different thread after loading the first so player gets in faster
//get other enemy types, improve AI so it can move and shoot
//make other AI types
//ok, make HUD feel more like a helmet with a HUD, show damage info
//go iron man style
//make it appear to curve like a helmet's face would

#include "../IMaterial.h"
//1. add air supplies
//2. show helmet damage
//3. indicators on hud, danger warnigns that blink!!!
//DEFINATELY MORE SPACEBUILD CONCEPTS
//ok, new idea CO-OP spacebuild related game
//survival like, kill wildlife, enemies, need to collect supplies for air and what not
void Renderer::Render(CCamera* cam, CRenderer* render)//if the renderable's parent is the same as the cameras, just render nicely
{
	//todo: update per frame constant buffer here when we have one
	PROFILE("RendererProcess");
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

	Vec3 worldcampos = globalview.GetTranslation();

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
			r->Render(cam, &renderqueue);

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
	if (this->_shadows)
	{
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
			tmpCamera._far = shadowMappingSplitDepths[cascade_i];
			if (cascade_i > 0)
				tmpCamera._near = shadowMappingSplitDepths[cascade_i - 1];

			//tmpCamera.doProjection();
			tmpCamera.doMatrix();

			//build basic frustum that fits the tmpCamera
			CCamera culling;
			BuildShadowFrustum(&tmpCamera, culling, this->dirToLight);

			//AABB frustumBB = tmpCamera.GetFrustumAABB();
			//const BOX &frustumBB = tmpCamera.GetMatrices().GetFrustumBox();
			//scene.ListObjectsIntersectingSweptBox(objs, frustumBB, g_DirToLight);
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
		/*if (showdebug > 1)
		{
		//draw shadow maps
		renderer->SetTexture(0,this->shadowMapViews[0]);
		Rect r(300,600,0,300);
		renderer->DrawRect(&r,0xFFFFFFF);

		if (this->shadowSplits > 1)
		{
		renderer->SetTexture(0,this->shadowMapViews[1]);
		r = Rect(300,600,300,600);
		renderer->DrawRect(&r,0xFFFFFFF);
		}
		if (this->shadowSplits > 2)
		{
		renderer->SetTexture(0,this->shadowMapViews[2]);
		r = Rect(0,300,0,300);
		renderer->DrawRect(&r,0xFFFFFFF);
		}
		if (this->shadowSplits > 3)
		{
		renderer->SetTexture(0,this->shadowMapViews[3]);
		r = Rect(0,300,300,600);
		renderer->DrawRect(&r,0xFFFFFFF);
		}
		}*/
	}

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
		renderer->SetCullmode(CULL_CW);

		//renderer->SetPixelTexture(0, renderer->terrain_texture);

		for (int li = 0; li < shadowSplits; li++)
			renderer->SetPixelTexture(li + 1, this->shadowMapViews[li]);

		//setup shadow filters
		//if (showdebug > 0)
		//for (int i = 0; i < 3; i++)
		renderer->context->PSSetSamplers(3, 1, &this->shadowSampler_linear);
		//else
		//for (int i = 0; i < 3; i++)
		//renderer->context->PSSetSamplers(3, 1, &this->shadowSampler);
	}

	//ok, lets be dumb and update materials here
	for (auto ii : IMaterial::GetList())
	{
		ii.second->Update(renderer);
	}

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

	/* 4. RENDER OBJECTS */
	GPUPROFILE2("Execute Commands");

	render->SetMatrix(VIEW_MATRIX, &globalview);
	Parent* last = 0;//keeps track of what view matix is active
	IMaterial* lastm = (IMaterial*)-1;
	for (int i = 0; i < renderqueue.size(); i++)
	{
		const RenderCommand& rc = renderqueue[i];
		//lets fill this with all the data we need, so we dont need
		//to go back to the source

		//goodbye to you
		/*if (last != rc.source->parent)//do I need to change the current view matrix?
		{
			if (rc.source->parent == parent)//object has same parent as the camera
			{
				render->SetMatrix(VIEW_MATRIX, &localview);
			}
			else if (rc.source->parent)//object has different parent
			{
				//need to recurse down to get correct matrix
				//goes from left to right so
				//temp = (matrix1*matrix2)*matrix3)*matrix4
				Matrix4 temp = rc.source->parent->mat.Inverse()*globalview;
				render->SetMatrix(VIEW_MATRIX, &temp);
			}
			else//object is in global space and has no parent
			{
				render->SetMatrix(VIEW_MATRIX, &globalview);
			}
			last = rc.source->parent;
		}*/

		//apply material settings
		bool shaderchange = false;
		if (lastm != rc.material)
		{
			rc.material->Apply(renderer);
			lastm = rc.material;
			shaderchange = true;
		}

		//todo: find applicable lights then set'em up
		int num_lights = 0;
		Light found_lights[3];//max per pixel

		//hack for the moment 
		const Vec3 position = rc.source ? rc.source->matrix.GetTranslation() : rc.position;
		const float radius = rc.source ? 10 : rc.radius;
		for (auto light : this->lights)
		{
			if (num_lights < 3 && light.position.dist(position) < light.radius + radius/*entity radius needs to go here*/)
			{
				//apply it
				found_lights[num_lights++] = light;
			}
		}
		
		//need to get right shader for right number of lights
		if (num_lights > 0 && rc.material->shader_builder)
		{
			//select right shader from material
			renderer->SetShader(rc.material->shader_lit_ptr);
			shaderchange = true;

			lastm = 0;
		}
		else
		{
			for (int i = 0; i < 3; i++)
				found_lights[i].radius = 0;
		}

		//for (int li = 0; li < shadowSplits; li++)
		//	renderer->SetPixelTexture(li + 1, this->shadowMapViews[li]);
		
		//only need to do this for shader builder shaders
		if (rc.material_instance.extra)
			renderer->SetPixelTexture(9, rc.material_instance.extra);

		/* execute render command */
		CShader* shader = renderer->shader;
		//normal mapping, sup
		//try and bring world shader into the normal shader system
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
			tmpCamera.perspectiveProjection();

			COLOR colors[] = { 0x14FF0000, 0x1400FF00, 0x140000FF };

			renderer->DrawFrustum(mv[6], tmpCamera._projectionMatrix, colors[i]);
		}
	}
}

void Renderer::UpdateUniforms(const RenderCommand* rc, const CShader* shader, const Matrix4* shadowMapTexXforms, bool shader_changed, const Light* light_list)
{
	//dirty hack
	if (shader->cbuffers.find("Terrain") != shader->cbuffers.end())
	{
		auto buffer = shader->cbuffers.find("Terrain")->second;

		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(buffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
		struct mdata
		{
			Matrix4 world;
			Matrix4 view;
			Matrix4 projection;
			Matrix4 wvp;
		};
		auto data = (mdata*)cb.pData;
		data->view = renderer->view;// _matrix;
		data->projection = renderer->projection;// cam->_projectionMatrix;
		data->world = Matrix4::Identity();
		auto wVP = Matrix4::Identity()*renderer->view*renderer->projection;
		wVP.MakeTranspose();
		data->wvp = wVP;
		renderer->context->Unmap(buffer.buffer, 0);

		renderer->context->VSSetConstantBuffers(buffer.vsslot, 1, &buffer.buffer);
		if (buffer.psslot >= 0)
			renderer->context->PSSetConstantBuffers(buffer.psslot, 1, &buffer.buffer);
	}

	//clean this up ok
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
		data->world = rc->source->matrix.Transpose();
		data->view = renderer->view.Transpose();
		data->projection = renderer->projection.Transpose();
		auto wVP = rc->source->matrix*renderer->view*renderer->projection;
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
		auto wVP = rc->source->matrix*renderer->view*renderer->projection;
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

	//calculate lighting
	//Vec3 lightdir = Vec3(0, 0, 0);
	/*if (rc->mesh.ib)
	{
	Vec3 pos = world.GetTranslation();
	if (rc->source->parent)
	{
	pos = rc->source->parent->LocalToWorld(pos);
	pos = (Vec3)(rc->source->parent->mat*Vec4(-pos.getnormal(), 0));
	lightdir = pos;
	}
	else
	{
	lightdir = -pos.getnormal();
	}
	lightdir = this->dirToLight;
	//set day and ambient lights

	//NEED TO FIX UP AMBIENT LIGHTING
	}*/
	//send a constant ambient/lighting value when shadows are on

	if (shader->buffers.lighting.buffer)
	{
		Vec3 dir = this->dirToLight;// lightdir;
		//if (rc->source->parent)
		//	dir = this->dirToLight;
		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(shader->buffers.lighting.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
#pragma pack(push)
#pragma pack(16)
		struct pl
		{
			Vec4 pos;
			Vec4 color;
			Vec4 radius;
		};
		struct mdata
		{
			Vec4 direction;
			Vec4 ambient;
			Vec4 daylight;
			pl lights[3];
		};
#pragma pack(pop)
		auto data = (mdata*)cb.pData;
		data->direction.xyz = dir;
		data->ambient = Vec4(0.2175f,0.2175,0.2175,0.2175);//rc->source->ambientlight;
		data->daylight = Vec4(0.9f, 0.9, 0.9, 0.9);;//rc->source->daylight;
		for (int i = 0; i < 3; i++)
		{
			data->lights[i].pos.xyz = light_list[i].position;
			data->lights[i].color.xyz = light_list[i].color;
			data->lights[i].radius.x = 30;// light_list[i].radius;
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
		int bones = static_cast<ObjModel*>(rc->source)->t->num_joints;
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
	//renderer->SetMatrix(VIEW_MATRIX, &globalview);
	//Parent* last = 0;//keeps track of what view matix is active
	//IMaterial* lastm = (IMaterial*)-1;
	std::vector<RenderCommand> renderqueue;

	r->Render(cam, &renderqueue);

	renderer->ApplyCam(cam);

	this->ProcessQueue(renderqueue);
}

void Renderer::ProcessQueue(const std::vector<RenderCommand>& renderqueue)
{
	Parent* last = 0;//keeps track of what view matix is active
	IMaterial* lastm = (IMaterial*)-1;

	for (int i = 0; i < renderqueue.size(); i++)
	{
		const RenderCommand& rc = renderqueue[i];
		//lets fill this with all the data we need, so we dont need
		//to go back to the source

		//apply material settings
		bool shader_changed = false;
		if (lastm != rc.material)
		{
			rc.material->Apply(renderer);
			lastm = rc.material;
			shader_changed = true;
		}

		//improve this, make it more static
		/* execute render command */
		CShader* shader = renderer->shader;
		//normal mapping, sup
		//try and bring world shader into the normal shader system
		Light found_lights[3];//max per pixel
		this->UpdateUniforms(&rc, shader, shadowMapTexXforms, shader_changed, found_lights);

		rc.mesh.vb->Bind();//maybe make this more data oriented?

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
