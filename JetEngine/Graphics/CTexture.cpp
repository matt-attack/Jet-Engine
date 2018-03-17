#include "CTexture.h"
#include "CRenderer.h"

#ifdef ANDROID
#include <jni.h>
extern JNIEnv* javaEnv;
extern jobject mattcraftrenderer;
#endif

//#include <d3dx11.h>

CTexture::CTexture(ID3D11ShaderResourceView* tex)
{
	texture_rv = tex;
	texture = 0;
	renderer->stats.textures++;
}

CTexture::~CTexture()
{
	if (this->texture_rv && this->texture_rv != renderer->GetMissingTextureImage())
	{
		texture_rv->Release();
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

inline std::wstring convertw(const std::string& as)
{
	wchar_t* buf = new wchar_t[as.size() * 2 + 2];
	swprintf(buf, L"%S", as.c_str());
	std::wstring rval = buf;
	delete[] buf;
	return rval;
}
#include <wincodec.h>
#include <DirectXTex/DirectXTex.h>
CTexture* CTexture::load_as_resource(const std::string &path, CTexture* res)
{

#ifdef _WIN32
	if (res->texture)
		renderer->stats.textures--;

	res->texture = 0;
	//D3DX11_IMAGE_LOAD_INFO info;
	//ZeroMemory(&info, sizeof(D3DX11_IMAGE_LOAD_INFO));
	//info.Format = DXGI_FORMAT_R8_UNORM;//DXGI_FORMAT_UNKNOWN;
	//info.MipLevels = 0;
	FILE* f = fopen(path.c_str(), "rb");
	if (f)// && path == "Content/Oxygen-Regular.ttf_sdf.png")
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		fclose(f);
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;
		IWICImagingFactory* fact;
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory2),
			reinterpret_cast<void **>(&fact)
		);
		DirectX::SetWICFactory(fact);
		DirectX::LoadFromWICFile(convertw(path).c_str(), 0, &metadata, image, 0);

		const DirectX::Image* i = image.GetImage(0, 0, 0);
		DirectX::ScratchImage mip_image;
		DirectX::GenerateMipMaps(*i, 0, 0, mip_image);
		D3D11_TEXTURE2D_DESC desc;
		desc.Format = i->format;
		desc.Height = i->height;
		desc.Width = i->width;
		desc.MipLevels = mip_image.GetMetadata().mipLevels;
		desc.ArraySize = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA data[15];
		for (int i = 0; i < desc.MipLevels; i++)
		{
			const auto im = mip_image.GetImage(i, 0, 0);
			data[i].pSysMem = im->pixels;
			data[i].SysMemPitch = im->rowPitch;
			data[i].SysMemSlicePitch = im->slicePitch;
		}
		renderer->device->CreateTexture2D(&desc, data, &res->texture);
		D3D11_SHADER_RESOURCE_VIEW_DESC vdesc;
		ZeroMemory(&vdesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		vdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		vdesc.Format = i->format;// DXGI_FORMAT_R32_FLOAT;// DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		vdesc.Texture2D.MipLevels = desc.MipLevels;
		renderer->device->CreateShaderResourceView(res->texture, &vdesc, &res->texture_rv);
		//renderer->device->CreateShaderResourceView
		HRESULT h = 0;// D3DX11CreateShaderResourceViewFromFileA(renderer->device, path.c_str(), 0, 0, &res->texture_rv, 0);
		if (FAILED(h))
			log("uhoh when loading texture");
	}
	//D3DX11CreateTextureFromFileA(renderer->device, path.c_str(), &info, 0, &res->texture, 0);
	if (res->texture_rv == 0)
	{
		//try again
		logf("Error: Loading texture %s failed\n", path.c_str());

		//insert dummy
		res->texture_rv = renderer->GetMissingTextureImage();
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
	data.SysMemSlicePitch = xRes * yRes * element_size;
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
		&tex->texture_rv
	);
	if (FAILED(hr2))
		throw 7;
	tex->texture = pTexture;
	pTexture->Release();
	return tex;
}