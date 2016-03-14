#ifndef DECAL_HEADER
#define DECAL_HEADER

#ifndef MATT_SERVER
#include "Graphics/CRenderer.h"
#include "Graphics/CVertexBuffer.h"
#include "Graphics\CTexture.h"

#include <vector>
#include <map>

#include <D3D11.h>

struct vert
{
	Vec3 pos;
	COLOR color;
	float u,v;
};

class DecalManager
{	
	CVertexBuffer vb;
	std::vector<vert> verts;//have a map of vectors? one for each material?
	int maxcount; int oldest;
	CShader* shader;
public:
	int count;
	DecalManager()
	{
		count = 0;
		maxcount = 100;
		oldest = 0;
		shader = renderer->CreateShader(4, "Shaders/model_diffuse.shdr");
	}

	~DecalManager()
	{

	}

	void AddDecal(char* texture, Vec3 pos, Vec3 tangent, Vec3 normal)
	{
		Vec3 right = tangent.cross(normal);
		pos -= tangent/2 + right/2;
		vert v = {pos,COLOR_ARGB(255,255,255,255),0.0f,0.0f};
		if (this->count < this->maxcount)
		{
			verts.push_back(v);
			v.pos = pos + right;
			v.u = 0.0f;
			v.v = 1.0f;
			verts.push_back(v);
			v.pos = pos + tangent + right;
			v.u = 1.0f;
			v.v = 1.0f;
			verts.push_back(v);

			v.pos = pos;
			v.u = 0.0f;
			v.v = 0.0f;
			verts.push_back(v);
			v.pos = pos + tangent;
			v.u = 1.0f;
			v.v = 0.0f;
			verts.push_back(v);
			v.pos = pos + tangent + right;
			v.u = 1.0f;
			v.v = 1.0f;
			verts.push_back(v);

			count++;
		}
		else
		{
			verts[oldest*6] = v;//verts.push_back(v);
			v.pos = pos + right;
			v.u = 0.0f;
			v.v = 1.0f;
			verts[oldest*6+1] = v;//verts.push_back(v);
			v.pos = pos + tangent + right;
			v.u = 1.0f;
			v.v = 1.0f;
			verts[oldest*6+2] = v;//verts.push_back(v);

			v.pos = pos;
			v.u = 0.0f;
			v.v = 0.0f;
			verts[oldest*6+3] = v;//verts.push_back(v);
			v.pos = pos + tangent;
			v.u = 1.0f;
			v.v = 0.0f;
			verts[oldest*6+4] = v;//verts.push_back(v);
			v.pos = pos + tangent + right;
			v.u = 1.0f;
			v.v = 1.0f;
			verts[oldest*6+5] = v;//verts.push_back(v);

			this->oldest++;
			this->oldest = oldest%this->maxcount;
		}
		this->vb.SetVertexDeclaration(renderer->GetVertexDeclaration(3));
		this->vb.Data(this->verts.data(), this->count*6*24, sizeof(vert));
	}

	void Draw()
	{
#ifdef ANDROID
		return;
#endif
		if (count > 0)
		{
			renderer->DepthWriteEnable(false);
			renderer->EnableAlphaBlending(true);
			renderer->SetPixelTexture(0, resources.get<CTexture>("decals.png"));
			renderer->SetCullmode(CULL_NONE);

			auto shader = renderer->SetShader(this->shader);

			if (shader->buffers.wvp.buffer)
			{
				auto wVP = Matrix4::Identity()*renderer->view*renderer->projection;
				wVP.MakeTranspose();
				
				D3D11_MAPPED_SUBRESOURCE cb;
				renderer->context->Map(shader->buffers.wvp.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);
				struct mdata
				{
					Matrix4 wvp;
				};
				auto data = (mdata*)cb.pData;
				data->wvp = wVP;
				renderer->context->Unmap(shader->buffers.wvp.buffer, 0);

				renderer->context->VSSetConstantBuffers(shader->buffers.wvp.vsslot,1,&shader->buffers.wvp.buffer);
			}
			vb.Bind();

			renderer->DrawPrimitive(PT_TRIANGLELIST, 0, count*6);
			renderer->EnableAlphaBlending(false);
			renderer->DepthWriteEnable(true);
		}
	}

	void Clear()
	{
		this->oldest = 0;
		this->count = 0;
		this->verts.clear();
	}
};

#endif
#endif