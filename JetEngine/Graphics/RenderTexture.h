#pragma once
#ifndef _RTEXTURE_HEADER
#define _RTEXTURE_HEADER

#include "CTexture.h"

#ifdef _WIN32
#include <d3dx11.h>
#endif

class ID3D11RenderTargetView;
class ID3D11DepthStencilView;
class CRenderTexture: public CTexture
{
	//ok, lets go with texture for the color, then texture_depth for the depth
public:
	bool created;
	friend class CRenderer;
	ID3D11RenderTargetView* color = 0;
	ID3D11DepthStencilView* depth = 0;

	ID3D11Texture2D* texture_depth = 0;

	ID3D11ShaderResourceView* depth_rv = 0;

	DXGI_FORMAT color_format;
public:

	CRenderTexture(ID3D11RenderTargetView* color, ID3D11DepthStencilView* depth);
	
	CRenderTexture(void);
	~CRenderTexture(void);

	static CRenderTexture* Create(int xRes, int yRes, DXGI_FORMAT color_format, DXGI_FORMAT depth_format);

	ID3D11ShaderResourceView* GetColorResourceView();

	ID3D11ShaderResourceView* GetDepthResourceView();

	void Clear(float a, float r, float g, float b);
};


#endif