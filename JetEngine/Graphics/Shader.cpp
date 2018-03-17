#include "Shader.h"
#include "CRenderer.h"
#include <D3D11.h>
//#include <D3DX11.h>
#include <fstream>

#include <D3Dcompiler.h>
#pragma comment (lib, "d3dcompiler.lib")

inline std::wstring convert(const std::string& as)
{
	wchar_t* buf = new wchar_t[as.size() * 2 + 2];
	swprintf(buf, L"%S", as.c_str());
	std::wstring rval = buf;
	delete[] buf;
	return rval;
}

CBuffer::CBuffer()
{
	this->buffer = 0;
	this->vsslot = this->psslot = this->gsslot = -1;
}

void CBuffer::UploadAndSet(void* data, int size)
{
	if (this->buffer)
	{
		D3D11_MAPPED_SUBRESOURCE cb;
		renderer->context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb);

		memcpy(cb.pData, data, size);

		renderer->context->Unmap(buffer, 0);

		if (psslot >= 0)
			renderer->context->PSSetConstantBuffers(psslot, 1, &buffer);
		if (vsslot >= 0)
			renderer->context->VSSetConstantBuffers(vsslot, 1, &buffer);
		if (gsslot >= 0)
			renderer->context->GSSetConstantBuffers(gsslot, 1, &buffer);
	}
}

CShader::CShader(const char* source, std::string filename, const char* vfunc, const char* pfunc, char** macros, char** macrodefinitions, int nummacros)
{
	this->vshader = 0;
	this->filename = filename;

	ID3D11VertexShader* vertexShader = NULL; //VS (NEW)

	D3D10_SHADER_MACRO* xmacros = 0;
	if (macros)
	{
		xmacros = new D3D10_SHADER_MACRO[nummacros + 1];
		for (int i = 0; i < nummacros; i++)
		{
			xmacros[i].Name = macros[i];
			xmacros[i].Definition = macrodefinitions[i];// "true";
		}
		xmacros[nummacros].Name = 0;
		xmacros[nummacros].Definition = 0;
	}
	//todo pass in file name here so includes function correctly and error information
	this->CompileVS(filename.c_str(), vfunc, xmacros, source);

	ID3D10Blob* gsBuf = 0;
	//if (gloc && gfunc)
	//	gsBuf = this->CompileGS(gloc, gfunc, xmacros);

	auto pixelShaderBuffer = this->CompilePS(filename.c_str(), pfunc, xmacros, source);

	this->SetupUniforms(vertexShaderBuffer, pixelShaderBuffer, gsBuf);
}

CShader::CShader(const char* vloc, const char* vfunc, const char* ploc, const char* pfunc, char** macros, char** macrodefinitions, int nummacros, const char* gloc, const char* gfunc)
{
#ifdef _WIN32
	this->filename = vloc;
	this->vshader = 0;

	ID3D11VertexShader* vertexShader = NULL; //VS (NEW)

	D3D10_SHADER_MACRO* xmacros = 0;
	if (macros)
	{
		xmacros = new D3D10_SHADER_MACRO[nummacros + 1];
		for (int i = 0; i < nummacros; i++)
		{
			xmacros[i].Name = macros[i];
			xmacros[i].Definition = macrodefinitions[i];// "true";
		}
		xmacros[nummacros].Name = 0;
		xmacros[nummacros].Definition = 0;
	}

	this->CompileVS(vloc, vfunc, xmacros);

	ID3D10Blob* gsBuf = 0;
	if (gloc && gfunc)
		gsBuf = this->CompileGS(gloc, gfunc, xmacros);

	auto pixelShaderBuffer = this->CompilePS(ploc, pfunc, xmacros);

	this->SetupUniforms(vertexShaderBuffer, pixelShaderBuffer, gsBuf);
#endif
}//load from file

ID3D10Blob* CShader::CompilePS(const char* file, const char* function, void* _macro, const char* str)
{
	D3D10_SHADER_MACRO* macro = (D3D10_SHADER_MACRO*)_macro;
	ID3D10Blob *pixelShaderBuffer;
	ID3D10Blob *errorMessage;
	//eof has a problem for some reason
	HRESULT result;
	if (str)
	{
		result = D3DCompile(str, strlen(str), file, macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, function, "ps_4_0", 0, 0, &pixelShaderBuffer, &errorMessage);
		//result = D3DX11CompileFromMemory(str, strlen(str), file, macro, 0, function, "ps_4_0", 0, 0, 0, &pixelShaderBuffer, &errorMessage, 0);
	}
	else
	{
		result = D3DCompileFromFile(convert(file).c_str(), macro/*defines*/,
			D3D_COMPILE_STANDARD_FILE_INCLUDE/*include*/,
			function, "ps_4_0", 0, 0, &pixelShaderBuffer, &errorMessage);
	}

	if (FAILED(result))
	{
		if (errorMessage != NULL)
		{
			MessageBoxA(GetActiveWindow(), (char*)errorMessage->GetBufferPointer(), "Invalid PS Code", MB_OK);
			errorMessage->Release();
			return 0;
		}
		else
		{
			std::string msg = "Could not find PS! ";
			msg += file;
			MessageBoxA(GetActiveWindow(), msg.c_str(), "PS Error", MB_OK);
			return 0;
		}
	}

	result = renderer->device->CreatePixelShader((DWORD*)pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), 0, &pshader);

	return pixelShaderBuffer;
}

ID3D10Blob* CShader::CompileVS(const char* file, const char* function, void* _macro, const char* str)
{
	D3D10_SHADER_MACRO* macro = (D3D10_SHADER_MACRO*)_macro;

	ID3D10Blob *errorMessage;

	HRESULT result;
	if (str)
		result = D3DCompile(str, strlen(str), file, macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, function, "vs_4_0", 0, 0, &vertexShaderBuffer, &errorMessage);
	else
	{
		result = D3DCompileFromFile(convert(file).c_str(), macro/*defines*/,
			D3D_COMPILE_STANDARD_FILE_INCLUDE/*include*/,
			function, "vs_4_0", 0, 0, &vertexShaderBuffer, &errorMessage);
	}

	if (FAILED(result))
	{
		if (errorMessage != NULL)
		{
			MessageBoxA(GetActiveWindow(), (char*)errorMessage->GetBufferPointer(), "Invalid VS Code", MB_OK);
			errorMessage->Release();
			return 0;
		}
		else
		{
			std::string msg = "Could not find VS! ";
			msg += file;
			MessageBoxA(GetActiveWindow(), msg.c_str() , "VS Error", MB_OK);
			return 0;
		}
	}

	result = renderer->device->CreateVertexShader((DWORD*)vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 0, &vshader);

	return vertexShaderBuffer;
}

ID3D10Blob* CShader::CompileGS(const char* file, const char* function, void* _macro, const char* str)
{
	D3D10_SHADER_MACRO* macro = (D3D10_SHADER_MACRO*)_macro;

	ID3D10Blob *geometryShaderBuffer;
	ID3D10Blob *errorMessage;

	HRESULT result;
	if (str)
		result = D3DCompile(str, strlen(str), file, macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, function, "gs_4_0", 0, 0, &geometryShaderBuffer, &errorMessage);
	else
		result = D3DCompileFromFile(convert(file).c_str(), macro/*defines*/,
		D3D_COMPILE_STANDARD_FILE_INCLUDE/*include*/,
		function, "gs_4_0", 0, 0, &geometryShaderBuffer, &errorMessage);

	if (FAILED(result))
	{
		if (errorMessage != NULL)
		{
			MessageBoxA(GetActiveWindow(), (char*)errorMessage->GetBufferPointer(), "Invalid VS Code", MB_OK);
			errorMessage->Release();
			return 0;
		}
		else
		{
			std::string msg = "Could not find GS! ";
			msg += file;
			MessageBoxA(GetActiveWindow(), msg.c_str(), "GS Error", MB_OK);
			return 0;
		}
	}

	result = renderer->device->CreateGeometryShader((DWORD*)geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), 0, &gshader);

	return geometryShaderBuffer;
}

/*void CShader::Bind()
{
renderer->context->VSSetShader(this->vshader, 0, 0);
renderer->context->PSSetShader(this->pshader, 0, 0);

this->BindIL(renderer->input_layout);
}*/

void CShader::BindIL(VertexDeclaration* il)
{
	//maybe cache the result?
	//if (il->elements == this->last_il)
	//return this->last_layout;

	auto lo = layouts.find(il->elements);
	if (lo == layouts.end())
	{
		ID3D11InputLayout* layout;
		//printf((char*)vertexShaderBuffer->GetBufferPointer());
		auto result = renderer->device->CreateInputLayout(il->elements, il->size, vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(), &layout);
		if (FAILED(result))
			throw 7;
		layouts[il->elements] = layout;
		renderer->context->IASetInputLayout(layout);
	}
	else
	{
		renderer->context->IASetInputLayout(lo->second);
	}
}


void CShader::SetupUniforms(ID3D10Blob* vsBuf, ID3D10Blob* psBuf, ID3D10Blob* gsBuf)
{
	ID3D10Blob* blobs[] = { vsBuf, psBuf, gsBuf };

	for (int n = 0; n < 3; n++)
	{
		ID3D10Blob* buf = blobs[n];
		if (buf == 0)
			continue;

		//reflect the shader
		ID3D11ShaderReflection* reflector;
		D3DReflect(buf->GetBufferPointer(), buf->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

		D3D11_SHADER_DESC desc;
		reflector->GetDesc(&desc);

		const char* type = n == 0 ? "VS" : (n == 1 ? "PS" : "GS");

		//ok, look for eieach constant buffer
		for (int i = 0; i < desc.ConstantBuffers; i++)
		{
			auto cbuf = reflector->GetConstantBufferByIndex(i);

			D3D11_SHADER_BUFFER_DESC cdesc;
			cbuf->GetDesc(&cdesc);
			//printf("%s Constant Buffer %d: %s, %d variables\n", type, i, cdesc.Name, cdesc.Variables);
			for (int vi = 0; vi < cdesc.Variables; vi++)
			{
				auto var = cbuf->GetVariableByIndex(vi);
				D3D11_SHADER_VARIABLE_DESC vdesc;
				var->GetDesc(&vdesc);
				//printf("Variable: %s\n", vdesc.Name);
			}

			//bind each buffer
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = cdesc.Size;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			// Create the constant buffer pointer so we can access the shader constant buffer from within this class.
			ID3D11Buffer* cbuffer;
			HRESULT result = renderer->device->CreateBuffer(&bufferDesc, NULL, &cbuffer);
			if (FAILED(result) || cbuffer == 0)
				throw 7;

			if (n == 0)
			{
				//v shader
				this->cbuffers[cdesc.Name] = CBuffer(i, -1, -1, cbuffer);
			}
			else if (n == 1)
			{
				//p shader
				auto& cb = this->cbuffers[cdesc.Name];
				cb.buffer = cbuffer;
				cb.psslot = i;
				cb.vsslot = cb.vsslot >= 0 ? cb.vsslot : -1;
			}
			else
			{
				//g shader
				auto& cb = this->cbuffers[cdesc.Name];
				cb.buffer = cbuffer;
				cb.gsslot = i;
				cb.psslot = cb.psslot >= 0 ? cb.psslot : -1;
				cb.vsslot = cb.vsslot >= 0 ? cb.vsslot : -1;
			}
		}
	}

	//1. general matrices
	//2. shadow parameters
	//3. skinning mats

	if (this->cbuffers.find("Matrices") != this->cbuffers.end())
		this->buffers.matrices = this->cbuffers["Matrices"];
	else
		this->buffers.matrices.buffer = 0;

	if (this->cbuffers.find("WVP") != this->cbuffers.end())
		this->buffers.wvp = this->cbuffers["WVP"];
	else
		this->buffers.wvp.buffer = 0;

	if (this->cbuffers.find("Shadow") != this->cbuffers.end())
		this->buffers.shadow = this->cbuffers["Shadow"];
	else
		this->buffers.shadow.buffer = 0;

	if (this->cbuffers.find("Lighting") != this->cbuffers.end())
		this->buffers.lighting = this->cbuffers["Lighting"];
	else
		this->buffers.lighting.buffer = 0;

	if (this->cbuffers.find("Skinning") != this->cbuffers.end())
		this->buffers.skinning = this->cbuffers["Skinning"];
	else
		this->buffers.skinning.buffer = 0;

	printf("shader loaded\n");
}


CShader* CShader::load_as_resource(const std::string &path, CShader* res)
{
	CShader* d = res;//new ModelData;

	std::ifstream file(path.c_str());

	char in[50];
	file.read(in, 23);
	in[23] = 0;

	if (strcmp(in, "#define geometry_shader") == 0)
		*d = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", 0, 0, 0, path.c_str(), "gs_main");
	//ok, here I need to load the first bit of the shader text and see what it says to determine what shader type it is
	//make a copy of uniforms
	else
		*d = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main");

	//new CShader("Content/Shaders/tree_billboards.shdr", "vs_main", "Content/Shaders/tree_billboards.shdr", "ps_main", 0, 0, "Content/Shaders/tree_billboards.shdr", "gs_main");

	return d;
}

void CShader::Reload(ResourceManager* mgr, const std::string& path)
{
#ifndef USEOPENGL

	CShader newshader;

	std::ifstream file(path.c_str());

	char in[50];
	file.read(in, 23);
	in[23] = 0;

	bool needs_gs = strcmp(in, "#define geometry_shader") == 0;
	if (needs_gs)
		newshader = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", 0, 0, 0, path.c_str(), "gs_main");
	//ok, here I need to load the first bit of the shader text and see what it says to determine what shader type it is
	//make a copy of uniforms
	else
		newshader = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main");

	//need to handle errors
	//auto newshader = CShader(filename.c_str(), "vs_main", filename.c_str(), "ps_main");
	if (newshader.vshader && newshader.pshader && (needs_gs == false || newshader.gshader))//makes sure that the new shader is valid
	{
		this->~CShader();//destruct me

#ifdef _DEBUG
#undef new
#undef DBG_NEW
#endif
		//and then just construct again here
		new (this) CShader();
#ifdef _DEBUG   
#ifndef DBG_NEW      
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )     
#define new DBG_NEW   
#endif
#endif

		*this = newshader;//CShader(filename.c_str(), "vs_main", filename.c_str(), "ps_main");
	}
#endif
}
