#ifndef _INDEXBUFFER_HEADER
#define _INDEXBUFFER_HEADER

//#include "CRenderer.h"
#include <D3D11.h>
//interface ID3D11Buffer;

class CIndexBuffer
{
#ifndef USEOPENGL
	ID3D11Buffer* obj;
#else
	GLuint obj;
#endif
public:
	unsigned int size;
	CIndexBuffer();
	~CIndexBuffer();

	void Bind();
	void Data(void* data, unsigned int length, int format);
};
#endif