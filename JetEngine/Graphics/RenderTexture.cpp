#include "RenderTexture.h"
#include "Renderable.h"
#include "CRenderer.h"

//#include <d3dx11.h>

CRenderTexture::CRenderTexture(void)
{
}


CRenderTexture::CRenderTexture(ID3D11RenderTargetView* color, ID3D11DepthStencilView* depth)
	:color(color), depth(depth)
{
	this->created = false;
}

CRenderTexture::~CRenderTexture(void)
{
	//if (color)
	//color->Release();
	//if (depth)
	//depth->Release();
}

void CRenderTexture::Clear(float a, float r, float g, float b)
{
#ifdef USEOPENGL
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	float color[4] = { r, g, b, a };
	if (this->color)
		renderer->context->ClearRenderTargetView(this->color, color);
	if (this->depth)
		renderer->context->ClearDepthStencilView(this->depth, D3D11_CLEAR_DEPTH, 1.0f, 0);
#endif
}

CRenderTexture* CRenderTexture::Create(int xRes, int yRes, DXGI_FORMAT color_format, DXGI_FORMAT depth_format, bool generate_mips)
{
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	// Setup the render target texture description.
	desc.Width = xRes;
	desc.Height = yRes;
	desc.MipLevels = generate_mips ? 0 : 1;
	desc.ArraySize = 1;
	desc.Format = color_format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = generate_mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	ID3D11Texture2D* renderTargetTexture = 0;
	if (color_format != DXGI_FORMAT_UNKNOWN)
	{
		// Create the render target texture.
		auto result = renderer->device->CreateTexture2D(&desc, NULL, &renderTargetTexture);
		if (FAILED(result))
		{
			throw 7;
		}
	}

	CRenderTexture* rt = new CRenderTexture;
	rt->created = true;
	
	if (depth_format != DXGI_FORMAT_UNKNOWN)
	{
		//this should be MUCH faster
		D3D11_TEXTURE2D_DESC shadowMapDesc;
		ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;// depth_format;// DXGI_FORMAT_R24G8_TYPELESS;
		shadowMapDesc.MipLevels = 1;
		shadowMapDesc.ArraySize = 1;
		shadowMapDesc.SampleDesc.Count = 1;
		shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		shadowMapDesc.Height = xRes;
		shadowMapDesc.Width = yRes;

		ID3D11Texture2D* depthTexture;
		HRESULT hr = renderer->device->CreateTexture2D(
			&shadowMapDesc,
			nullptr,
			&depthTexture
			);
		if (FAILED(hr))
			throw 7;

		rt->texture_depth = depthTexture;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = depth_format;// DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		ID3D11DepthStencilView* shadowDepthView;
		hr = renderer->device->CreateDepthStencilView(
			depthTexture,
			&depthStencilViewDesc,
			&shadowDepthView
			);
		if (FAILED(hr))
			throw 7;

		rt->depth = shadowDepthView;
	}
	else
	{
		rt->depth = 0;
	}

	ID3D11RenderTargetView* renderTargetView = 0;
	if (color_format != DXGI_FORMAT_UNKNOWN)
	{
		auto result = renderer->device->CreateRenderTargetView(renderTargetTexture, NULL, &renderTargetView);
		if (FAILED(result))
			throw 7;
	}

	rt->color_format = color_format;
	rt->color = renderTargetView;
	rt->texture = renderTargetTexture;
	rt->generate_mips = generate_mips;
	
	if (rt->color)
	{
		rt->texture_rv = rt->GetColorResourceView();
		rt->texture_rv->AddRef();
	}
	return rt;
}

ID3D11ShaderResourceView* CRenderTexture::GetColorResourceView()
{
	if (this->texture_rv)
	{
		this->texture_rv->AddRef();
		return this->texture_rv;
	}

	if (this->texture == 0)
		throw 7;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = color_format;// DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = generate_mips ? -1 : 1;

	//this is how to access the texture later
	ID3D11ShaderResourceView* resourceView;
	HRESULT hr = renderer->device->CreateShaderResourceView(
		texture,
		&shaderResourceViewDesc,
		&resourceView
		);
	if (FAILED(hr))
		throw 7;

	this->texture_rv = resourceView;
	return resourceView;
}

ID3D11ShaderResourceView* CRenderTexture::GetDepthResourceView()
{
	if (this->depth_rv)
	{
		this->depth_rv->AddRef();
		return this->depth_rv;
	}

	//todo, have some enums definiting types of depth textures
	//	32 bit float
	//	24 bit
	//	16 bit

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;// color_format;// DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	//this is how to access the texture later
	ID3D11ShaderResourceView* resourceView;
	HRESULT hr = renderer->device->CreateShaderResourceView(
		texture_depth,
		&shaderResourceViewDesc,
		&resourceView
		);
	if (FAILED(hr))
		throw 7;

	this->depth_rv = resourceView;
	return resourceView;
}
