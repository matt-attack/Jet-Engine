#ifndef MATT_SERVER
#ifndef VERTEX_BUFFER_HEADER
#define VERTEX_BUFFER_HEADER

struct D3D11_INPUT_ELEMENT_DESC;
struct VertexDeclaration
{
	D3D11_INPUT_ELEMENT_DESC* elements;
	int size;
};

enum class VertexBufferUsage
{
	Dynamic,//upload and read many times
	Static,//upload once, read many times
};

#ifndef _WIN32
#ifdef ANDROID
#include <GLES2\gl2.h>
#endif

class CVertexBuffer
{
	unsigned int size;
	unsigned int stride;
public:
	GLuint obj;
	CVertexBuffer()
	{
		size = 0;
		obj = 0;
	}

	~CVertexBuffer()
	{
		if (this->obj)
			glDeleteBuffers(1,&this->obj);
	}

	unsigned int GetSize()
	{
		return this->size;
	}

	unsigned int GetStride()
	{
		return this->stride;
	}

	void Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, obj);
	}

	void Data( const void* data, size_t length, unsigned long usage );

	inline void SubData( const void* data, size_t offset, size_t length )
	{
		void* pVoid;

		//vb->Lock(offset, length, &pVoid, 0);
		//memcpy(pVoid, data, length);
		//vb->Unlock();

		glBindBuffer( GL_ARRAY_BUFFER, obj );
		glBufferSubData( GL_ARRAY_BUFFER, offset, length, data );
		glBindBuffer( GL_ARRAY_BUFFER, 0);
	}

	inline void GetSubData( void* data, size_t offset, size_t length )
	{
		//void* pVoid;

		//vb->Lock(offset, length, &pVoid, D3DLOCK_READONLY);
		//memcpy(data,pVoid,length);
		//vb->Unlock();

		//glBindBuffer( GL_ARRAY_BUFFER, obj );
		//glGetBufferSubData( GL_ARRAY_BUFFER, offset, length, data );
	}
};
#else

class ID3D11Buffer;

class CVertexBuffer
{
	unsigned int size;
	unsigned int stride;
public:
	
	friend class CRenderer;
	ID3D11Buffer* vb;
	VertexDeclaration vd;

private:
	VertexBufferUsage type;

public:
	
	CVertexBuffer(VertexBufferUsage vb_type)
	{
		size = 0;
		vb = 0;
		vd.elements = 0;
		vd.size = 0;
		type = vb_type;
	}

	CVertexBuffer()
	{
		size = 0;
		vb = 0;
		//vd. = 0;
		type = VertexBufferUsage::Dynamic;
	}

	~CVertexBuffer();

	unsigned int GetSize()
	{
		return this->size;
	}

	unsigned int GetStride()
	{
		return this->stride;
	}

	//need way to cache the formats, so there are not duplicates

	void SetVertexDeclaration(VertexDeclaration vd)
	{
		this->vd = vd;
	}

	void Bind();

	void Data(const void* data, size_t length, unsigned int stride);

	void DataNoResize(const void* data, size_t length, unsigned int stride);

	void SubData(const void* data, size_t offset, size_t length);

	inline void GetSubData( void* data, size_t offset, size_t length )
	{
		void* pVoid=0;

		//vb->Lock(offset, length, &pVoid, D3DLOCK_READONLY);
		//memcpy(data,pVoid,length);
		//vb->Unlock();
		throw 7;
		//glBindBuffer( GL_ARRAY_BUFFER, obj );
		//glGetBufferSubData( GL_ARRAY_BUFFER, offset, length, data );
	}

	//creates it, but does not add any data
	void Allocate(size_t length, unsigned int stride);
};
#endif
#endif
#endif