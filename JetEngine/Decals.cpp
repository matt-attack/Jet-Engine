#include "Decals.h"
#include "Graphics/Shader.h"

void DecalManager::AddDecal(char* texture, Vec3 pos, Vec3 tangent, Vec3 normal)
{
	Vec3 right = tangent.cross(normal);
	pos -= tangent/2 + right/2;
	vert v = { pos, COLOR_ARGB(255, 255, 255, 255), 0.0f, 0.0f };
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
		verts[oldest * 6] = v;//verts.push_back(v);
		v.pos = pos + right;
		v.u = 0.0f;
		v.v = 1.0f;
		verts[oldest * 6 + 1] = v;//verts.push_back(v);
		v.pos = pos + tangent + right;
		v.u = 1.0f;
		v.v = 1.0f;
		verts[oldest * 6 + 2] = v;//verts.push_back(v);

		v.pos = pos;
		v.u = 0.0f;
		v.v = 0.0f;
		verts[oldest * 6 + 3] = v;//verts.push_back(v);
		v.pos = pos + tangent;
		v.u = 1.0f;
		v.v = 0.0f;
		verts[oldest * 6 + 4] = v;//verts.push_back(v);
		v.pos = pos + tangent + right;
		v.u = 1.0f;
		v.v = 1.0f;
		verts[oldest * 6 + 5] = v;//verts.push_back(v);

		this->oldest++;
		this->oldest = oldest%this->maxcount;
	}

	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	this->vb.SetVertexDeclaration(renderer->GetVertexDeclaration(elm3, 3));
	this->vb.Data(this->verts.data(), this->count * 6 * 24, sizeof(vert));
}


void DecalManager::Draw()
{
#ifdef ANDROID
	return;
#endif
	if (count > 0)
	{
		renderer->DepthWriteEnable(false);
		renderer->EnableAlphaBlending(true);
		
		renderer->SetPixelTexture(0, this->texture);
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

			renderer->context->VSSetConstantBuffers(shader->buffers.wvp.vsslot, 1, &shader->buffers.wvp.buffer);
		}
		vb.Bind();

		renderer->DrawPrimitive(PT_TRIANGLELIST, 0, count * 6);
		renderer->EnableAlphaBlending(false);
		renderer->DepthWriteEnable(true);
	}
}