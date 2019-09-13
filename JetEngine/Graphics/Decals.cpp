#include "Decals.h"
#include "Shader.h"

void DecalManager::AddDecal(char* texture, Vec3 pos, Vec3 tangent, Vec3 normal)
{
	Vec3 right = tangent.cross(normal);
	const float scale = 4;
	tangent *= scale;
	right *= scale;

	pos -= tangent/2 + right/2;
	vert v = { pos, COLOR_ARGB(255, 255, 255, 255), 0.0f, 0.0f };
	mutex_.lock();
	if (count_ < max_count_)
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

		count_++;
	}
	else
	{
		verts[oldest_ * 6] = v;//verts.push_back(v);
		v.pos = pos + right;
		v.u = 0.0f;
		v.v = 1.0f;
		verts[oldest_ * 6 + 1] = v;//verts.push_back(v);
		v.pos = pos + tangent + right;
		v.u = 1.0f;
		v.v = 1.0f;
		verts[oldest_ * 6 + 2] = v;//verts.push_back(v);

		v.pos = pos;
		v.u = 0.0f;
		v.v = 0.0f;
		verts[oldest_ * 6 + 3] = v;//verts.push_back(v);
		v.pos = pos + tangent;
		v.u = 1.0f;
		v.v = 0.0f;
		verts[oldest_ * 6 + 4] = v;//verts.push_back(v);
		v.pos = pos + tangent + right;
		v.u = 1.0f;
		v.v = 1.0f;
		verts[oldest_ * 6 + 5] = v;//verts.push_back(v);

		oldest_++;
		oldest_ = oldest_%max_count_;
	}
	dirty_ = true;
	mutex_.unlock();
}


#include "../camera.h"
void DecalManager::Draw(const CCamera* cam)
{
	mutex_.lock();
	if (dirty_)
	{
		VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
		{ ELEMENT_COLOR, USAGE_COLOR },
		{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

		vb_.SetVertexDeclaration(renderer->GetVertexDeclaration(elm3, 3));
		vb_.Data(this->verts.data(), count_ * 6 * 24, sizeof(vert));
		dirty_ = false;
	}
	mutex_.unlock();

	// todo this probably shouldnt be here
	if (!shader_)
	{
		shader_ = resources.get_shader("Shaders/model_diffuse.shdr");
		texture_ = resources.get_unsafe<CTexture>("decals.png");
	}
	
	if (count_ > 0)
	{
		renderer->DepthWriteEnable(false);
		renderer->EnableAlphaBlending(true);
		
		renderer->SetPixelTexture(0, texture_);
		renderer->SetCullmode(CULL_NONE);

		auto shader = renderer->SetShader(shader_);

		if (shader->buffers.wvp.buffer)
		{
			auto wVP = Matrix4::Identity()*cam->_matrix*cam->_projectionMatrix;// *renderer->view*renderer->projection;
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
		vb_.Bind();

		renderer->DrawPrimitive(PT_TRIANGLELIST, 0, count_ * 6);
		renderer->EnableAlphaBlending(false);
		renderer->DepthWriteEnable(true);
	}
}