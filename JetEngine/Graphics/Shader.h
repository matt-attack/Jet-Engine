#ifndef _SHADER_HEADER
#define _SHADER_HEADER

#include "../ResourceManager.h"
#include <map>
#include <string>

class ID3D11Buffer;
class VertexDeclaration;

//shader constant buffer wrapper
struct CBuffer
{
	int vsslot; int psslot; int gsslot;
	ID3D11Buffer* buffer;

	CBuffer();
	CBuffer(int vss, int pss, int gss, ID3D11Buffer* b) : vsslot(vss), psslot(pss), buffer(b), gsslot(gss) { }

	void UploadAndSet(void* data, int size);
};

interface ID3D11InputLayout;
interface ID3D11PixelShader;
interface ID3D11VertexShader;
interface ID3D11GeometryShader;
interface ID3D10Blob;

struct UniformBuffers
{
	CBuffer skinning;
	CBuffer lighting;
	CBuffer shadow;
	CBuffer wvp;
	CBuffer matrices;
};

struct D3D11_INPUT_ELEMENT_DESC;


class CShader: public Resource
{
	friend class CRenderer;

public:
#ifdef USEOPENGL
	int program;
	VertexElement* elements;
	int Constants[10];
#else
	ID3D11VertexShader* vshader = 0;
	ID3D11PixelShader* pshader = 0;
	ID3D11GeometryShader* gshader = 0;
private:
	ID3D10Blob* vertexShaderBuffer;//used for generation of layouts
#endif

	std::map<D3D11_INPUT_ELEMENT_DESC*, ID3D11InputLayout*> layouts;

	ID3D10Blob* CompileVS(const char* filename, const char* function, void* macro, const char* str = 0);
	ID3D10Blob* CompilePS(const char* filename, const char* function, void* macro, const char* str = 0);
	ID3D10Blob* CompileGS(const char* filename, const char* function, void* macro, const char* str = 0);

public:
	
	UniformBuffers buffers;
	std::map<std::string, CBuffer> cbuffers;
	
	CShader() {};
	CShader(const char* vloc, const char* vfunc, const char* ploc, const char* pfunc, char** macros = 0, char** macrodefinitions = 0, int nummacros = 0, const char* gloc = 0, const char* gfunc = 0);
	CShader(const char* source, std::string filename, const char* vfunc, const char* pfunc, char** macros = 0, char** macrodefinitions = 0, int nummacros = 0);

	~CShader()
	{
#ifndef USEOPENGL
		//if (this->shader)
		//shader->Release();

		//if (this->pshader)
		//pshader->Release();
		//vertexShaderBuffer->Release();
#else
		//glDeleteShader(program);
#endif
	}

	void BindIL(VertexDeclaration* il);

	std::string filename;

private:

	void SetupUniforms(ID3D10Blob* vertexTable, ID3D10Blob* pixelTable, ID3D10Blob* geometryTable = 0);

public:
	//if you load a shader by name and do "#define geometry_shader" on the first line it will compile on a geometry shader as well
	static CShader* load_as_resource(const std::string &path, CShader* res);

	virtual void Reload(ResourceManager* mgr, const std::string& filename);
};

#endif