#include "CTexture.h"
#include "CRenderer.h"

#ifdef ANDROID
#include <jni.h>
extern JNIEnv* javaEnv;
extern jobject mattcraftrenderer;
#endif

extern ID3D11ShaderResourceView* missing_texture;

CTexture::CTexture(ID3D11ShaderResourceView* tex)
{
	texture = tex;
	data = 0;
	renderer->stats.textures++;
}

CTexture::~CTexture()
{
	if (this->texture && this->texture != missing_texture)
	{
		texture->Release();
		renderer->stats.textures--;
	}
}

void CTexture::Reload(ResourceManager* mgr, const std::string& filename)
{
	CTexture *ptr = this;
	ptr->~CTexture();
#ifdef _DEBUG
#undef new
#undef DBG_NEW
#endif
	//and then just construct again here
	new (ptr)CTexture();
#ifdef _DEBUG   
#ifndef DBG_NEW      
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )     
#define new DBG_NEW   
#endif
#endif

	CTexture *rv = CTexture::load_as_resource(filename, ptr);
}

CTexture* CTexture::load_as_resource(const std::string &path, CTexture* res)
{

#ifdef _WIN32
	if (res->texture)
		renderer->stats.textures--;

	res->texture = 0;
	D3DX11_IMAGE_LOAD_INFO info;
	ZeroMemory(&info, sizeof(D3DX11_IMAGE_LOAD_INFO));
	info.Format = DXGI_FORMAT_R8_UNORM;//DXGI_FORMAT_UNKNOWN;
	info.MipLevels = 0;
	HRESULT h = D3DX11CreateShaderResourceViewFromFileA(renderer->device, path.c_str(), 0, 0, &res->texture, 0);
	if (FAILED(h))
		printf("uhoh");
	//D3DX11CreateTextureFromFileA(renderer->device, path.c_str(), &info, 0, &res->texture, 0);
	if (res->texture == 0)
	{
		//try again
		printf("texture loading failed\n");

		//insert dummy
		res->texture = missing_texture;
	}
	else
	{
		//renderer->stats.texture_mem += res->texture->
		renderer->stats.textures++;
	}
#else
	//log2("started trying to load");
	int t = 0;
	jclass ren = javaEnv->GetObjectClass(mattcraftrenderer);
	if (ren)
	{
		//log2("got obj class");
		jmethodID mid = javaEnv->GetMethodID(ren, "loadNamedTexture", "(Ljava/lang/String;)I");
		if (mid)
		{
			//log2("got MID");
			//need to get rid of everything past the .
			char t2[50];
			int i;
			for (i = 0; i < path.length(); i++)
			{
				if (path[i] == '.')
					break;
				else
					t2[i] = path[i];
			}
			t2[i] = 0;


			jstring javaString = (jstring)javaEnv->NewStringUTF((const char *)t2);//path.c_str());

			t = javaEnv->CallIntMethod(mattcraftrenderer, mid, javaString);
			if (t == -1)
			{
				logf("MISSING TEXTURE: %s", t2);
				res->texture = 0;
			}
			else
				res->texture = t;
		}
	}
#endif
	return res;
}

CTexture* CTexture::Create(int xRes, int yRes, DXGI_FORMAT format, const char* in_data)
{
	CTexture* tex = new CTexture();

	D3D11_TEXTURE2D_DESC desc1;
	desc1.Width = xRes;
	desc1.Height = yRes;
	desc1.MipLevels = desc1.ArraySize = 1;
	desc1.Format = format;// DXGI_FORMAT_R32_FLOAT;// DXGI_FORMAT_R8G8B8A8_UNORM;
	desc1.SampleDesc.Count = 1;
	desc1.SampleDesc.Quality = 0;
	desc1.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_IMMUTABLE;//generally dont need anything different with a texture D3D11_USAGE_DYNAMIC;
	desc1.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc1.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc1.MiscFlags = 0;

	ID3D11Texture2D *pTexture = NULL;

	int element_size = 4;//95% of the time
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = in_data;// this->heights;
	data.SysMemPitch = xRes * 4;
	data.SysMemSlicePitch = xRes*yRes * element_size;
	auto hr3 = renderer->device->CreateTexture2D(&desc1, &data, &pTexture);
	if (FAILED(hr3))
		throw 7;

	D3D11_SHADER_RESOURCE_VIEW_DESC htshaderResourceViewDesc;
	ZeroMemory(&htshaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	htshaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	htshaderResourceViewDesc.Format = format;// DXGI_FORMAT_R32_FLOAT;// DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	htshaderResourceViewDesc.Texture2D.MipLevels = 1;

	//this is how to access the texture later
	ID3D11ShaderResourceView* htresourceView;
	auto hr2 = renderer->device->CreateShaderResourceView(
		pTexture,
		&htshaderResourceViewDesc,
		&tex->texture
		);
	if (FAILED(hr2))
		throw 7;
	tex->data = pTexture;
	pTexture->Release();
	return tex;
}