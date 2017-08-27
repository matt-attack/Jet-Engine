#include "CVertexBuffer.h"
#include "CRenderer.h"
#ifndef _WIN32
#include <pthread.h>
#endif

#include <D3D11.h>

#ifdef _WIN32
CVertexBuffer::~CVertexBuffer()
{
	if (vb)
	{
		vb->Release();
		renderer->stats.vertexbuffer_mem -= this->size;
		renderer->stats.vertexbuffers--;
	}
	//glDeleteBuffers(1,&this->vb);
}
#endif

#ifdef _WIN32
void CVertexBuffer::Data(const void* data, size_t length, unsigned int stride)
{
	this->stride = stride;

	//release the old one if it exists and is a different size
	if (vb && type == VertexBufferUsage::Static)
	{
		printf("Can't reupload data");
		throw 7;
	}
	if (vb && length != size)
	{
		vb->Release();
		renderer->stats.vertexbuffer_mem -= this->size;
		renderer->stats.vertexbuffers--;
		vb = 0;
	}

	if (vb == 0)
	{
		// Set up the description of the static vertex buffer.
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = length;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		if (type == VertexBufferUsage::Dynamic)
		{
			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;// D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
		}
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = data;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		auto result = renderer->device->CreateBuffer(&vertexBufferDesc, &vertexData, &vb);
		if (FAILED(result))
			throw 7;

		this->size = length;

		renderer->stats.vertexbuffer_mem += this->size;
		renderer->stats.vertexbuffers++;
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE res;
		renderer->context->Map(vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		//res.pData = (void*)data;
		memcpy(res.pData, data, size);
		renderer->context->Unmap(vb, 0);
		//renderer->context->UpdateSubresource(vb, 0, 0, data, 0, 0);
	}
}

void CVertexBuffer::Allocate(size_t length, unsigned int stride)
{
	this->stride = stride;

	if (vb)
	{
		printf("already allocated\n");
		throw 7;
	}

	if (vb == 0)
	{
		// Set up the description of the static vertex buffer.
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = length;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		if (type == VertexBufferUsage::Dynamic)
		{
			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;// D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
		}
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = 0;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		auto result = renderer->device->CreateBuffer(&vertexBufferDesc, &vertexData, &vb);
		if (FAILED(result))
		{
			throw 7;
		}

		this->size = length;

		renderer->stats.vertexbuffer_mem += this->size;
		renderer->stats.vertexbuffers++;
	}
	else
	{
		throw 7;
	}
}

void CVertexBuffer::DataNoResize(const void* data, size_t length, unsigned int stride)
{
	this->stride = stride;

	if (length > this->size)
	{
		printf("CVertexBuffer: length greater than size\n");
		throw 7;
	}

	if (vb == 0)
	{
		// Set up the description of the static vertex buffer.
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = length;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		if (type == VertexBufferUsage::Dynamic)
		{
			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;// D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
		}
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = data;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		auto result = renderer->device->CreateBuffer(&vertexBufferDesc, &vertexData, &vb);
		if (FAILED(result))
		{
			throw 7;
		}

		this->size = length;

		renderer->stats.vertexbuffer_mem += this->size;
		renderer->stats.vertexbuffers++;
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE res;
		renderer->context->Map(vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		//res.pData = (void*)data;
		memcpy(res.pData, data, length);
		renderer->context->Unmap(vb, 0);
		//renderer->context->UpdateSubresource(vb, 0, 0, data, 0, 0);
	}
}

void CVertexBuffer::SubData(const void* data, size_t offset, size_t length)
{
	D3D11_MAPPED_SUBRESOURCE res;
	renderer->context->Map(vb, 0, D3D11_MAP_WRITE, 0, &res);
	memcpy((char*)res.pData+offset, data, length);
	renderer->context->Unmap(vb, 0);
}

void CVertexBuffer::Bind() const
{
	renderer->SetVertexDeclaration(this->vd);
	const unsigned int offset = 0;
	renderer->context->IASetVertexBuffers(0, 1, &this->vb, &stride, &offset);
	//renderer->context->IASetInputLayout(this->vd);//renderer->SetVertexDeclaration(this->vd);

	//renderer->d3ddev->SetStreamSource(0,this->vb,0,this->stride);
	//glBindBuffer( GL_ARRAY_BUFFER, obj );
}
#else

void CVertexBuffer::Data( const void* data, size_t length, unsigned long usage )
{
	if (renderer->thread != pthread_self())
		log("whoopsie");

	if (this->obj != 0)
	{
		glDeleteBuffers(1, &this->obj);
		this->obj = 0;
	}
	//if (this->obj == 0)
	glGenBuffers(1, &this->obj);

	glBindBuffer( GL_ARRAY_BUFFER, obj );
	glBufferData( GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	this->size = length;
}
#endif