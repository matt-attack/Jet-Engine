#include "CIndexBuffer.h"
#include "CRenderer.h"

#include <D3D11.h>

CIndexBuffer::CIndexBuffer()
{
	obj = 0;
	size = 0;
}

CIndexBuffer::~CIndexBuffer()
{
#ifndef USEOPENGL
	if (obj)
		obj->Release();
#else
	if (obj)
		glDeleteBuffers(1,&this->obj);
#endif
}

void CIndexBuffer::Bind()
{
#ifndef USEOPENGL
	renderer->context->IASetIndexBuffer(obj, DXGI_FORMAT::DXGI_FORMAT_R16_UINT, 0);
#else
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj);
#endif
}

void CIndexBuffer::Data(void* data, unsigned int length, int format)
{
#ifndef USEOPENGL
	if (obj)
	{
		obj->Release();
		obj = 0;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = length;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = data;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	auto result = renderer->device->CreateBuffer(&indexBufferDesc, &indexData, &obj);
	if(FAILED(result))
	{
		throw 7;
	}

	size = length;
#else
	if (renderer->thread != pthread_self())
		log("whoopsie");

	glGenBuffers(1,&this->obj);//oops, fix threading

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	size = length;
#endif
}