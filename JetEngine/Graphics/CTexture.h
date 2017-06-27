#ifndef _TEXTURE_HEADER
#define _TEXTURE_HEADER

#ifdef _WIN32
#include <d3dx11.h>
#endif

#include "../ResourceManager.h"

class CTexture: public Resource
{
	friend class CRenderer;

public:
	int check = 0x55555555;
	ID3D11ShaderResourceView* texture;
	ID3D11Texture2D* data;

	CTexture()
	{
		texture = 0;
		data = 0;
	}

	CTexture(ID3D11ShaderResourceView* tex);

	~CTexture();

	operator ID3D11ShaderResourceView*()
	{
		return this->texture;
	}

	static CTexture* Create(int xRes, int yRes, DXGI_FORMAT format, const char* data = 0);

	virtual void Reload(ResourceManager* mgr, const std::string& filename);

	static CTexture* load_as_resource(const std::string &path, CTexture* res);
};
#endif