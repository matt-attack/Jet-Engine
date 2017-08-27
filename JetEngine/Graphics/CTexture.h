#ifndef _TEXTURE_HEADER
#define _TEXTURE_HEADER

#include "../ResourceManager.h"


struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

#ifdef _WIN32
#include <D3D11.h>
#endif

class CTexture: public Resource
{
	friend class CRenderer;

public:
	int check = 0x55555555;
	ID3D11ShaderResourceView* texture_rv;
	ID3D11Texture2D* texture;
	CTexture()
	{
		texture = 0;
		texture_rv = 0;
	}

	CTexture(ID3D11ShaderResourceView* tex);

	~CTexture();

	operator ID3D11ShaderResourceView*()
	{
		return this->texture_rv;
	}

	static CTexture* Create(int xRes, int yRes, DXGI_FORMAT format, const char* data = 0);

	virtual void Reload(ResourceManager* mgr, const std::string& filename);

	static CTexture* load_as_resource(const std::string &path, CTexture* res);
};
#endif