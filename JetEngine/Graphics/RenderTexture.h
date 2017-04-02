#pragma once
#include "CTexture.h"

class CRenderTexture: public CTexture
{
public:
	bool created;
	friend class CRenderer;
	ID3D11RenderTargetView* color;
	ID3D11DepthStencilView* depth;
	ID3D11Texture2D* color_texture;
	ID3D11Texture2D* depth_texture;

	ID3D11ShaderResourceView* depth_rv = 0;

	DXGI_FORMAT color_format;
public:

	CRenderTexture(ID3D11RenderTargetView* color, ID3D11DepthStencilView* depth);
	//CRenderTexture(
	CRenderTexture(void);
	~CRenderTexture(void);

	static CRenderTexture* Create(int xRes, int yRes, DXGI_FORMAT color_format, DXGI_FORMAT depth_format);

	ID3D11ShaderResourceView* GetColorResourceView();

	ID3D11ShaderResourceView* GetDepthResourceView();

	void Clear(float a, float r, float g, float b);
};

