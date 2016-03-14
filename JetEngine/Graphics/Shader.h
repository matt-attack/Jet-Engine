#ifndef _SHADER_HEADER
#define _SHADER_HEADER

#include "../ResourceManager.h"
#include <map>
#include <string>



struct Uniform
{
	int type;
	char* name;
	int vec4count;//the size
#ifndef USEOPENGL
	int vslocation;
	int pslocation;
#else
	int location;
#endif
};

class ID3D11Buffer;
class VertexDeclaration;

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

	//int viewMatInv, viewProjMatInv, viewerPos;
	//int worldNormalMat;
	//int skinMatRows;
	//int lightPos;
};

struct D3D11_INPUT_ELEMENT_DESC;
//struct D3D10_SHADER_MACRO;
//struct D3D_SHADER_MACRO;

//struct _D3D_SHADER_MACRO;
//need to make sure to handle if vertex shader has these as well, check if id's are different
class CShader: public Resource
{
	friend class CRenderer;

public:
#ifdef USEOPENGL
	int program;
	VertexElement* elements;
	int Constants[10];
#else

	ID3D11VertexShader* vshader;
	ID3D11PixelShader* pshader;
	ID3D11GeometryShader* gshader;

	ID3D10Blob* vertexShaderBuffer;//used for generation of layouts
#endif

	std::map<D3D11_INPUT_ELEMENT_DESC*, ID3D11InputLayout*> layouts;

	void BindIL(VertexDeclaration* il);

	ID3D10Blob* CompileVS(const char* file, const char* function, void* macro);
	ID3D10Blob* CompilePS(const char* file, const char* function, void* macro);
	ID3D10Blob* CompileGS(const char* file, const char* function, void* macro);

public:
	UniformBuffers buffers;
	std::map<std::string, CBuffer> cbuffers;
	//void Bind();implement me

	CShader() {};
	CShader(const char* vs, const char* ps);
	CShader(const char* vloc, const char* vfunc, const char* ploc, const char* pfunc, char** macros = 0, int nummacros = 0, const char* gloc = 0, const char* gfunc = 0);
	
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

#ifndef USEOPENGL
	void SetupUniforms(ID3D10Blob* vertexTable, ID3D10Blob* pixelTable, ID3D10Blob* geometryTable = 0);
#else
	void SetupUniforms()
	{
		std::map<std::string, int> binds;
		binds["WorldViewProjection"] = CONSTANT_WORLDVIEWPROJECTION;
		binds["View"] = CONSTANT_VIEW;
		binds["World"] = CONSTANT_WORLD;
		binds["Projection"] = CONSTANT_PROJECTION;
		binds["ViewProjection"] = CONSTANT_VIEWPROJECTION;
		binds["LightDirection"] = CONSTANT_LIGHTDIRECTION;
		binds["Time"] = CONSTANT_TIME;

		for (int i = 0; i < 10; i++)
		{
			this->Constants[i] = -1;
		}

		for (auto& ii: binds)
		{
			int loc = glGetUniformLocation(program, ii.first.c_str());
			if (loc != -1)
				this->Constants[ii.second] = loc;
		}

		int total = -1;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &total);
		for (int i = 0; i < total; i++)
		{
			int name_len = -1, num = -1;
			GLenum type = GL_ZERO;
			char name[100];
			glGetActiveUniform(program, GLuint(i), sizeof(name)-1, &name_len, &num, &type, name);
			name[name_len] = 0;
			GLuint location = glGetUniformLocation(program, name);

			int unif_size = 1;
			if (type == GL_FLOAT_MAT4)
				unif_size = 4;
			else if (type == GL_FLOAT_MAT3)
				unif_size = 3;
			else if (type == GL_FLOAT_MAT2)
				unif_size = 2;

			Uniform u;// = new Uniform;
			u.location = location;//uniform_desc.RegisterIndex;
			u.vec4count = unif_size;//uniform_desc.RegisterCount;
			u.type = type;// uniform_desc.Type;
			this->uniforms[name] = u;

			logf("Got Uniform: %s size %d at %d\n", name, u.vec4count, location);
		}
	}
#endif

	static CShader* load_as_resource(const std::string &path, CShader* res);

	virtual void Reload(ResourceManager* mgr, const std::string& filename);
};

#endif