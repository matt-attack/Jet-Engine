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
	ID3D11ShaderResourceView* texture;

	CTexture()
	{
		texture = 0;
	}

	CTexture(ID3D11ShaderResourceView* tex);

	~CTexture();

	operator ID3D11ShaderResourceView*()
	{
		return this->texture;
	}

	virtual void Reload(ResourceManager* mgr, const std::string& filename);

	static CTexture* load_as_resource(const std::string &path, CTexture* res);
};
#endif