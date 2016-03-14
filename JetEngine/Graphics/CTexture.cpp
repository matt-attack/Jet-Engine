#include "CTexture.h"
#include "CRenderer.h"

#ifdef ANDROID
#include <jni.h>
extern JNIEnv* javaEnv;
extern jobject mattcraftrenderer;
#endif

CTexture::CTexture(ID3D11ShaderResourceView* tex)
{
	texture = tex;
	renderer->stats.textures++;
}

CTexture::~CTexture()
{
	if (this->texture && this->texture != renderer->missing_texture)
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
	new (ptr) CTexture();
#ifdef _DEBUG   
#ifndef DBG_NEW      
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )     
#define new DBG_NEW   
#endif
#endif

	CTexture *rv = CTexture::load_as_resource(filename,ptr);
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
		res->texture = renderer->missing_texture;
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
