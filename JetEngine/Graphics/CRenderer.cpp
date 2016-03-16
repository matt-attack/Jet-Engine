#include "CRenderer.h"
#include "CVertexBuffer.h"
#include "Shader.h"
#include "../ResourceManager.h"
#include "../ModelData.h"
#include "font.h"
#include <D3D11.h>

int msaa_count = 1;// 2;
int msaa_quality = 0;
VertexDeclaration CRenderer::GetVertexDeclaration(VertexElement* elm, unsigned int count)
{
	int key = 0;
	for (int i = 0; i < count; i++)
	{
		key += (elm[i].type + elm[i].usage)*i * 23;
	}

	//look for a matching key
	for (int i = 0; i < this->vaos.size(); i++)
	{
		if (vaos[i].key == key && count == vaos[i].size && vaos[i].data[0].type == elm[0].type)
		{
			//double check it
			return vaos[i].vd;
		}
	}
	//ok, so go through and make some sort of hash and use a multimap to store it
	//if we already have it, just return that, else create it, cache, and return
	//rename this to GetVertexDeclaration and remove the other function
	//if (this->vertexdeclarations[id].elements)
	//	throw 7;

	auto elements = new D3D11_INPUT_ELEMENT_DESC[count];

	//copy the setup data
	auto elmcopy = new VertexElement[count];
	memcpy(elmcopy, elm, sizeof(VertexElement)*count);

	unsigned int currentpos = 0;
	for (int i = 0; i < count; i++)
	{
		auto type = elm[i].type;
		auto usage = elm[i].usage;

		DXGI_FORMAT t = DXGI_FORMAT_R32G32B32A32_FLOAT;

		if (type == ELEMENT_FLOAT)
			t = DXGI_FORMAT_R32_FLOAT;// D3DDECLTYPE_FLOAT1;
		else if (type == ELEMENT_FLOAT2)
			t = DXGI_FORMAT_R32G32_FLOAT;// D3DDECLTYPE_FLOAT2;
		else if (type == ELEMENT_FLOAT3)
			t = DXGI_FORMAT_R32G32B32_FLOAT;//D3DDECLTYPE_FLOAT3;
		else if (type == ELEMENT_FLOAT4)
			t = DXGI_FORMAT_R32G32B32A32_FLOAT;
		else if (type == ELEMENT_COLOR)
			t = DXGI_FORMAT_B8G8R8A8_UNORM;// D3DDECLTYPE_D3DCOLOR;
		else if (type == ELEMENT_UBYTE4)
			t = DXGI_FORMAT_R8G8B8A8_UINT;//  D3DDECLTYPE_UBYTE4;

		elements[i].Format = t;
		if (i == 0)
			elements[i].AlignedByteOffset = 0;
		else
			elements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		if (usage == USAGE_POSITION)
			elements[i].SemanticName = "POSITION";
		else if (usage == USAGE_TEXCOORD)
			elements[i].SemanticName = "TEXCOORD";
		else if (usage == USAGE_COLOR)
			elements[i].SemanticName = "COLOR";
		else if (usage == USAGE_BLENDINDICES)
			elements[i].SemanticName = "BLENDINDICES";
		else if (usage == USAGE_BLENDWEIGHT)
			elements[i].SemanticName = "BLENDWEIGHT";
		else if (usage == USAGE_NORMAL)
			elements[i].SemanticName = "NORMAL";
		else if (usage == USAGE_TANGENT)
			elements[i].SemanticName = "TANGENT";
		else
			elements[i].SemanticName = "uh";
		elements[i].SemanticIndex = 0;

		elements[i].InputSlot = 0;
		elements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elements[i].InstanceDataStepRate = 0;

		if (usage == USAGE_NONE)
		{

		}
	}


	//todo: make copy of elements
	this->vaos.push_back({ elmcopy, count, key, { elements, count } });

	//this->vertexdeclarations[id].elements = elements;
	//this->vertexdeclarations[id].size = count;

	return{ elements, count };
	/*polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);*/

}

//Custom vertex
struct TLVERTEX
{
	float x;
	float y;
	float z;
	float rhw;
	COLOR color;
	float u;
	float v;
};

CRenderer::CRenderer()
{
	this->wireframe = false;
#ifndef USEOPENGL
	this->context = 0;
	this->device = 0;
#endif

	this->vsync = false;

	this->stats.vertexbuffers = 0;
	this->stats.vertexbuffer_mem = 0;
	this->stats.textures = 0;
	this->stats.texture_mem = 0;
	this->font = 0;

	this->pos = 0;
	for (int i = 0; i < 200; i++)
	{
		this->values[i].value = 0.0f;
		this->values[i].text = false;
	}

	this->Override = false;
	this->_wvpDirty = false;
	this->shader = 0;
	for (int i = 0; i < 25; i++)
		this->shaders[i] = 0;
}

CRenderer::~CRenderer()
{
	//free font
	if (this->font)
		delete this->font;

	//swap->Release();
	device->Release();
	context->Release();

	bs_additive->Release();
	bs_alpha->Release();
}

void CRenderer::GetViewport(Viewport* vp)
{
#ifndef USEOPENGL
	unsigned int viewports = 1;
	D3D11_VIEWPORT viewport;
	context->RSGetViewports(&viewports, &viewport);
	/*D3DVIEWPORT9 viewport;
	//renderer->d3ddev->GetViewport(&viewport);*/
	vp->Height = viewport.Height;
	vp->Width = viewport.Width;
	vp->X = viewport.TopLeftX;
	vp->Y = viewport.TopLeftY;
	vp->MaxZ = viewport.MaxDepth;
	vp->MinZ = viewport.MinDepth;
#else
	vp->Height = this->yres;
	vp->Width = this->xres;
	vp->X = 0;
	vp->Y = 0;
	vp->MaxZ = 1.0f;
	vp->MinZ = 0.0f;
#endif
}

void CRenderer::SetViewport(Viewport* vp)
{
#ifndef USEOPENGL
	unsigned int viewports = 1;
	D3D11_VIEWPORT viewport;
	viewport.Height = vp->Height;
	viewport.Width = vp->Width;
	viewport.TopLeftX = vp->X;
	viewport.TopLeftY = vp->Y;
	viewport.MaxDepth = vp->MaxZ;
	viewport.MinDepth = vp->MinZ;
	this->context->RSSetViewports(1, &viewport);
#else
	vp->Height = this->yres;
	vp->Width = this->xres;
	vp->X = 0;
	vp->Y = 0;
	vp->MaxZ = 1.0f;
	vp->MinZ = 0.0f;
#endif
}

#include "CTexture.h"

void CRenderer::SetPixelTexture(int stage, CTexture* tex)
{
	if (tex && tex->texture == 0)
		throw 7;
	if (tex)
		context->PSSetShaderResources(stage, 1, &tex->texture);
	//throw 7;
	if (tex)
		this->current_texture = tex->texture;
	else
		this->current_texture = 0;
#ifdef USEOPENGL
	glBindTexture(GL_TEXTURE_2D, tex->texture);
#else
	//d3ddev->SetTexture(stage, tex->texture); 
#endif
};

void CRenderer::SetVertexTexture(int stage, CTexture* tex)
{
	if (tex)
		context->VSSetShaderResources(stage, 1, &tex->texture);
	//throw 7;
	if (tex)
		this->current_texture = tex->texture;
	else
		this->current_texture = 0;
#ifdef USEOPENGL
	glBindTexture(GL_TEXTURE_2D, tex->texture);
#else
	//d3ddev->SetTexture(stage, tex->texture); 
#endif
};

void CRenderer::SetPixelTexture(int stage, ID3D11ShaderResourceView* tex)
{
	//throw 7;
	this->current_texture = tex;
#ifdef USEOPENGL
	glBindTexture(GL_TEXTURE_2D, tex);
#else

	this->context->PSSetShaderResources(stage, 1, &tex);
	//this->context->VSSetShaderResources(stage, 1, &tex);
#endif
};

void CRenderer::SetVertexTexture(int stage, ID3D11ShaderResourceView* tex)
{
	//throw 7;
	this->current_texture = tex;
#ifdef USEOPENGL
	glBindTexture(GL_TEXTURE_2D, tex);
#else

	//this->context->PSSetShaderResources(stage, 1, &tex);
	this->context->VSSetShaderResources(stage, 1, &tex);
#endif
};


void CRenderer::SetFilter(int stage, FilterMode mode)
{
#ifndef USEOPENGL
	if (mode == FilterMode::Linear)
		context->PSSetSamplers(0, 1, &linear_sampler);
	else
		context->PSSetSamplers(0, 1, &point_sampler);
	//this->d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, (DWORD)mode);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
#endif
};

void CRenderer::SetSrcBlend(Blend mode)
{
	//this->d3ddev->SetRenderState(D3DRS_SRCBLEND, mode);
}

void CRenderer::SetDestBlend(Blend mode)
{
	//this->d3ddev->SetRenderState(D3DRS_DESTBLEND, mode);
}

void CRenderer::EnableAlphaBlending(bool yes)
{
#ifndef USEOPENGL
	float blendfactor[4] = { 1, 1, 1, 1 };
	if (yes)
		context->OMSetBlendState(bs_alpha, blendfactor, 0xFFFFFFFF);
	else
		context->OMSetBlendState(bs_solid, blendfactor, 0xFFFFFFFF);
	//d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, yes);
#else
	if (yes)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
#endif
}

void CRenderer::SetCullmode(enum CullMode mode)
{
#ifndef USEOPENGL
	if (this->wireframe == false)
	{
		if (mode == CullMode::CULL_NONE)
			context->RSSetState(rs_none);
		else if (mode == CullMode::CULL_CW)
			context->RSSetState(rs_cw);
		else if (mode == CullMode::CULL_CCW)
			context->RSSetState(rs_ccw);
	}
	else
		context->RSSetState(rs_wireframe);
#else
	if (this->Override) return;
	if (mode == CULL_NONE)
		glDisable(GL_CULL_FACE);
	else if (mode == CULL_CW)
	{
		//glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}
	else
	{
		//glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	//glEnable(GL_DEPTH_TEST);

	//glCullFace(GL_BACK);
	//glFrontFace(GL_CW);
#endif
}

void CRenderer::SetCullmode(enum CullMode mode, bool oride)
{
	throw 7;
#ifndef USEOPENGL
	//renderer->d3ddev->SetRenderState(D3DRS_CULLMODE, mode); 
#else
	this->SetCullmode(mode);
#endif
	this->Override = oride;
};

void CRenderer::StencilFunc(unsigned int func, unsigned int ref, unsigned int mask)
{
#ifndef USEOPENGL
	//d3ddev->SetRenderState(D3DRS_STENCILFUNC, mask);
	//d3ddev->SetRenderState(D3DRS_STENCILREF, ref);
	//d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, mask);
#endif
}

#ifndef USEOPENGL
void CRenderer::StencilOp(int fail, int zfail, int zpass)
{
	throw 7;
	//d3ddev->SetRenderState(D3DRS_STENCILFAIL, fail);
	//d3ddev->SetRenderState(D3DRS_STENCILZFAIL, zfail);
	//d3ddev->SetRenderState(D3DRS_STENCILPASS, zpass);
}

inline void CRenderer::StencilMask(unsigned int mask)
{
	throw 7;
	//d3ddev->SetRenderState(D3DRS_STENCILMASK, mask); 
}
void CRenderer::DepthWriteEnable(bool on)
{
	if (on)
		context->OMSetDepthStencilState(depthStencilState, 1);
	else
		context->OMSetDepthStencilState(depthStencilStateNoWrite, 1);

	//d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, on); 
}
#endif

void CRenderer::SetDepthRange(float Near, float Far)
{
#ifndef _WIN32
	glDepthRangef(Near*2.0f - 1.0f,Far);//should this be -1.0 -> 1.0f?
#else
	D3D11_VIEWPORT vp;
	unsigned int vps = 1;
	this->context->RSGetViewports(&vps, &vp);
	vp.MinDepth = Near;
	vp.MaxDepth = Far;
	this->context->RSSetViewports(1, &vp);
#endif
}

void CRenderer::SetFont(char* name, int size)
{
	if (this->font == 0)
	{
		this->font = new Font;
		this->font->Load(0, 0, 0, 0, 0);
	}
	this->fontsize = size;
}


CShader* CRenderer::CreateShader(int id, const char* name)
{
	if (this->shaders[id])
		printf("Recreating/Overwriting shader %i\n", id);
	//throw 7;
	this->shaders[id] = resources.get<CShader>(name);
	return this->shaders[id];
}

#ifdef _WIN32
void CRenderer::Init(HWND hWnd, int scrx, int scry)
#else
void CRenderer::Init(int scrx, int scry)
#endif
{
	this->xres = scrx;
	this->yres = scry;

	DXGI_SWAP_CHAIN_DESC swapdesc;
	swapdesc.Windowed = true;
	swapdesc.Flags = 0;
	swapdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapdesc.BufferCount = 1;
	swapdesc.OutputWindow = hWnd;
	swapdesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//MSAA
	swapdesc.SampleDesc.Count = msaa_count;// 1;
	swapdesc.SampleDesc.Quality = msaa_quality;// 0;


	swapdesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	swapdesc.BufferDesc.Height = scry;
	swapdesc.BufferDesc.Width = scrx;
	swapdesc.BufferDesc.RefreshRate.Numerator = 60;//??
	swapdesc.BufferDesc.RefreshRate.Denominator = 1;
	swapdesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapdesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	IDXGIFactory* factory;
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

	factory->MakeWindowAssociation(hWnd, 0);

	IDXGIAdapter* adapter = 0;
	int i = 0;
	printf("Graphics Adapters:\n");
	while (true)
	{
		auto tmp = adapter;
		factory->EnumAdapters(i++, &adapter);
		if (adapter == 0)
		{
			adapter = tmp;
			break;
		}

		DXGI_ADAPTER_DESC adapterdesc;
		adapter->GetDesc(&adapterdesc);

		printf("%ls Dedicated: %d MB Shared: %d\n", adapterdesc.Description, adapterdesc.DedicatedVideoMemory / 1024 / 1024, adapterdesc.SharedSystemMemory / 1024 / 1024);

		//enumerate outputs
		int oi = 0;
		IDXGIOutput* adapterOutput = 0;
		do
		{
			adapter->EnumOutputs(oi++, &adapterOutput);
			if (adapterOutput)
			{
				DXGI_OUTPUT_DESC desc;
				adapterOutput->GetDesc(&desc);
				printf("    Output: %ls %dx%d\n", desc.DeviceName, desc.DesktopCoordinates.right - desc.DesktopCoordinates.left, desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);
			}
		} while (adapterOutput);

		if (i == 2)
			break;
		//do stuff
	}

	factory->EnumAdapters(0, &adapter);//just use first one for now

	DXGI_ADAPTER_DESC adapterdesc;
	adapter->GetDesc(&adapterdesc);

	IDXGIOutput* adapterOutput;
	adapter->EnumOutputs(0, &adapterOutput);

	auto level = D3D_FEATURE_LEVEL_11_0;
	unsigned int flags = 0;
#ifdef _DEBUG
	//flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	auto res = D3D11CreateDeviceAndSwapChain(
		0, D3D_DRIVER_TYPE_HARDWARE, NULL/*software*/,
		flags/*flags*/, &level/*feature levels*/, 1/*num feature levels*/,
		D3D11_SDK_VERSION,
		&swapdesc,
		&chain, &device, NULL, &context);//outputs
	if (context == 0 || device == 0 || FAILED(res))
	{
		printf("DX11 Initialization Failed\n");
	}

	if (msaa_count > 1)
	{
		UINT a;
		device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 2, &a);
	}

	D3D11_VIEWPORT viewport;
	viewport.Height = scry;
	viewport.Width = scrx;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;
	viewport.TopLeftX = viewport.TopLeftY = 0;
	context->RSSetViewports(1, &viewport);

	// Get the pointer to the back buffer.
	ID3D11Resource* backBufferPtr;
	chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);

	// Create the render target view with the back buffer pointer.
	//ID3D11RenderTargetView* renderTargetView;
	HRESULT result = device->CreateRenderTargetView(backBufferPtr, NULL, &renderTargetView);
	if (FAILED(result))
		throw 7;

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	//create depth buffer
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory((void*)&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = scrx;
	depthBufferDesc.Height = scry;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = msaa_count;
	depthBufferDesc.SampleDesc.Quality = msaa_quality;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	ID3D11Texture2D* depthStencilBuffer;
	result = device->CreateTexture2D(&depthBufferDesc, 0, &depthStencilBuffer);
	if (FAILED(result))
		throw 7;

	//create the view
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//ID3D11DepthStencilState* depthStencilState;
	result = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(result))
		throw 7;

	context->OMSetDepthStencilState(depthStencilState, 1);

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	result = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilStateNoWrite);
	if (FAILED(result))
		throw 7;

	//then create view for depth
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = msaa_count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
	if (FAILED(result))
		throw 7;

	depthStencilBuffer->Release();

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);


	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;//D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = msaa_count > 1 ? true : false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	ID3D11RasterizerState* rasterState;
	result = device->CreateRasterizerState(&rasterDesc, &rs_none);
	if (FAILED(result))
		throw 7;

	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	result = device->CreateRasterizerState(&rasterDesc, &rs_wireframe);
	if (FAILED(result))
		throw 7;

	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	result = device->CreateRasterizerState(&rasterDesc, &rs_cw);
	if (FAILED(result))
		throw 7;

	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	result = device->CreateRasterizerState(&rasterDesc, &rs_ccw);
	if (FAILED(result))
		throw 7;

	// Now set the rasterizer state.
	context->RSSetState(rs_none);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	device->CreateBlendState(&blendStateDescription, &bs_alpha);

	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	device->CreateBlendState(&blendStateDescription, &bs_solid);

	//additive and subtractive blending
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	device->CreateBlendState(&blendStateDescription, &bs_additive);


	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;// D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	device->CreateBlendState(&blendStateDescription, &bs_subtractive);

	float blendfactor[4] = { 1, 1, 1, 1 };
	context->OMSetBlendState(bs_solid, blendfactor, 0xFFFFFFFF);


	D3D11_SAMPLER_DESC sd;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	sd.MaxAnisotropy = 1;
	sd.MipLODBias = 0.0f;
	//sd.Filter = D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_ANISOTROPIC;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	result = device->CreateSamplerState(&sd, &linear_sampler);
	if (FAILED(result))
		throw 7;

	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	result = device->CreateSamplerState(&sd, &point_sampler);

	context->PSSetSamplers(0, 1, &linear_sampler);

	//renderer shaders
	renderer->CreateShader(16, "Shaders/guitexture.txt");
	renderer->CreateShader(15, "Shaders/gui.txt");

	VertexElement elm8[] = { { ELEMENT_FLOAT4, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };
	this->rectangle_v_buffer.SetVertexDeclaration(this->GetVertexDeclaration(elm8, 3));


	//use 32 bit depth stencil
#ifndef USEOPENGL

	this->SetFont("Arial", 20);

	this->gui_texture = resources.get<CTexture>("controls.png");

	//D3DX11_IMAGE_LOAD_INFO info;
	//ZeroMemory(&info, sizeof(D3DX11_IMAGE_LOAD_INFO));
	//info.Format = DXGI_FORMAT_UNKNOWN;
	//info.MipLevels = 4;
	//this->terrain_texture = new CTexture;
	//HRESULT h = D3DX11CreateShaderResourceViewFromFileA(renderer->device, "texture.png", 0, 0, &this->terrain_texture->texture, 0);
	//if (FAILED(h))
	//	printf("uhoh");
	/*HRESULT h = D3DXCreateTextureFromFileExA(d3ddev,
	"texture.png",
	D3DX_DEFAULT,
	D3DX_DEFAULT,
	4,
	0,
	D3DFMT_UNKNOWN,
	D3DPOOL_MANAGED,
	D3DX_FILTER_LINEAR,
	D3DX_FILTER_POINT,//point works better with alpha, but looks wierd
	D3DCOLOR_XRGB(255, 255, 255),//makes white clear
	NULL,
	NULL,
	&terrain_texture);*/
	this->missing_texture = resources.get<CTexture>("missing.png")->texture;

	this->passthrough = new CShader("Content/Shaders/passthrough.shdr", "vs_main", "Content/Shaders/passthrough.shdr", "ps_main");
	this->unlit_textured = new CShader("Content/Shaders/unlit_texture.shdr", "vs_main", "Content/Shaders/unlit_texture.shdr", "ps_main");
#else
	this->thread = pthread_self();
	char* fragmentSource = "precision mediump float;"
		"varying vec2 Texcoord;"
		"uniform vec4 color;"
		"uniform sampler2D texture;"
		"void main(void) {"
		"gl_FragColor = texture2D(texture, Texcoord) * color;"//vec4(1,0,1,1);"//"gl_FragColor = texture2D(texture, Texcoord);"
		"}";

	char* vertexSource =  "precision mediump float;"
		"attribute vec4 pos;"
		"varying vec2 Texcoord;"
		"void main() {"
		"Texcoord = pos.zw;"
		"gl_Position = vec4( pos.xy, 0, 1 );"
		"}";

	/*GLint shader = */this->CreateShader(13, vertexSource, fragmentSource);

	//this->SetVDSize(8,2);
	//this->AddVDElement(8,ELEMENT_FLOAT3,USAGE_POSITION);
	//this->AddVDElement(8,ELEMENT_COLOR,USAGE_COLOR);

	char* fragmentSource2 = "precision mediump float;"
		"uniform vec4 color;"
		"void main(void) {"
		"gl_FragColor = color;"//vec4(1,0,1,1);"//"gl_FragColor = texture2D(texture, Texcoord);"
		"}";

	char* vertexSource2 = "precision mediump float;"
		"attribute vec2 pos;"
		"void main() {"
		"gl_Position = vec4( pos.xy, 0, 1 );"
		"}";

	/*GLint shader2 = */this->CreateShader(14, vertexSource2, fragmentSource2);


	//entity shader 2
	char* fragmentSource5 = "precision mediump float;"
		"varying vec2 Texcoord;"
		"uniform sampler2D texture;"
		"uniform float light;"
		"void main(void) {"
		"gl_FragColor = texture2D(texture, Texcoord)*light;"
		"}";

	char* vertexSource5 = "precision mediump float;"
		"attribute vec3 pos;"
		"attribute vec2 tex;"
		"attribute vec3 normal;"
		//"attribute 
		//"uniform mat3x4 mats;"
		"varying vec2 Texcoord;"
		//"varying float light";
		"uniform mat4 WorldViewProjection;"
		"void main() {"
		"Texcoord = tex;"
		"gl_Position = WorldViewProjection * vec4( pos.xyz, 1 );"
		"}";

	/*GLint EntityShader2 = */this->CreateShader(9, vertexSource5, fragmentSource5);

	//this->SetVDSize(6,3);
	//this->AddVDElement(6,ELEMENT_FLOAT3,USAGE_POSITION);
	//this->AddVDElement(6,ELEMENT_FLOAT2,USAGE_TEXCOORD);
	//this->AddVDElement(6,ELEMENT_FLOAT3,USAGE_NORMAL);

	//this->SetVDSize(9,5);
	//this->AddVDElement(9,ELEMENT_FLOAT3,USAGE_POSITION);
	//this->AddVDElement(9,ELEMENT_FLOAT3,USAGE_NORMAL);
	//this->AddVDElement(9,ELEMENT_FLOAT2,USAGE_TEXCOORD);
	//this->AddVDElement(9,ELEMENT_UBYTE4,USAGE_NONE);
	//this->AddVDElement(9,ELEMENT_UBYTE4,USAGE_NONE);

	char* ps = "precision mediump float;"
		"varying vec2 Texcoord;"
		"uniform sampler2D texture;"
		"uniform float light;"
		"void main(void) {"
		"gl_FragColor = vec4(1,1,1,1);"//*light;"//texture2D(texture, Texcoord)*light;"
		"}";
	char* vs = //"#version 120;"
		"precision mediump float;"
		"attribute vec3 pos;"
		"attribute vec2 tex;"
		"attribute vec3 normal;"
		"attribute vec4 boneWeights;"
		"attribute vec4 boneIndices;"
		"uniform mat4 bonemats[60];"
		"varying vec2 Texcoord;"
		"uniform mat4 WorldViewProjection;"
		"void main() {"
		"float weight1 = float(boneWeights.x)/255.0;"
		"float weight2 = float(boneWeights.y)/255.0;"
		"float weight3 = float(boneWeights.z)/255.0;"
		"float weight4 = float(boneWeights.w)/255.0;"
		"mat4 m = bonemats[int(boneIndices.x)] * weight1;"//(boneWeights.y/255);"//mat4(0.5);"//vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),vec4(0,0,0,1));"////bonemats[1];"//int(boneIndices.x)];"// * boneWeights.x/255;"
		"m += bonemats[int(boneIndices.y)] * weight2;"
		"m += bonemats[int(boneIndices.z)] * weight3;"
		"m += bonemats[int(boneIndices.w)] * weight4;"
		"Texcoord = tex;"
		"gl_Position = WorldViewProjection * (m * vec4( pos.xyz, 1));"
		"}";
	this->CreateShader(6, vs, ps);

	//this->SetVDSize(17,5);
	//this->AddVDElement(17,ELEMENT_FLOAT3,USAGE_POSITION);
	//this->AddVDElement(17,ELEMENT_FLOAT3,USAGE_NORMAL);
	//this->AddVDElement(17,ELEMENT_FLOAT2,USAGE_TEXCOORD);
	//this->AddVDElement(17,ELEMENT_UBYTE4,USAGE_BLENDWEIGHT);
	//this->AddVDElement(17,ELEMENT_UBYTE4,USAGE_BLENDINDICES);

	//star shader
	char* fragmentSource8 = "precision mediump float;"
		//"varying vec2 Texcoord;"
		//"uniform sampler2D texture;"
		"uniform float light;"
		"void main(void) {"
		"gl_FragColor = vec4(1,1,1,light);"//texture2D(texture, Texcoord)*light;"
		"}";

	char* vertexSource8 = "precision mediump float;"
		"attribute vec3 pos;"
		//"attribute vec2 tex;"

		//"varying vec2 Texcoord;"
		"uniform mat4 WorldViewProjection;"
		"void main() {"
		//"Texcoord = tex;"
		"gl_Position = WorldViewProjection * vec4( pos.xyz, 1 );"
		"}";

	/*GLint StarShader = */this->CreateShader(3, vertexSource8, fragmentSource8);
	starunif = glGetUniformLocation(this->shaders[3]->program, "light");
	//this->SetVDSize(3,2);
	//this->AddVDElement(3,ELEMENT_FLOAT3,USAGE_POSITION);
	//this->AddVDElement(3,ELEMENT_COLOR,USAGE_COLOR);

	//entity shader
	char* fragmentSource3 = "precision mediump float;"
		"varying vec2 Texcoord;"
		"uniform sampler2D texture;"
		"uniform float light;"
		"void main(void) {"
		"gl_FragColor = texture2D(texture, Texcoord)*light;"
		"}";

	char* vertexSource3 = "precision mediump float;"
		"attribute vec3 pos;"
		"attribute vec2 tex;"
		"varying vec2 Texcoord;"
		"uniform mat4 WorldViewProjection;"
		"void main() {"
		"Texcoord = tex;"
		"gl_Position = WorldViewProjection * vec4( pos.xyz, 1 );"
		"}";

	/*GLint EntityShader = */this->CreateShader(12, vertexSource3, fragmentSource3);
	this->CreateShader(4, vertexSource3, fragmentSource3);

	//blockmodel color only shader
	char* fragmentSource11 = "precision mediump float;"
		//"varying vec2 Texcoord;"
		"varying vec4 ocolor;"
		//"uniform sampler2D texture;"
		"uniform float light;"
		"void main(void) {"
		"gl_FragColor = ocolor;"
		"}";

	char* vertexSource11 = "precision mediump float;"
		"attribute vec3 pos;"
		"attribute vec4 color;"

		"varying vec4 ocolor;"

		//"varying vec2 Texcoord;"
		"uniform mat4 WorldViewProjection;"
		"void main() {"
		"ocolor = color;"
		//"Texcoord = tex;"
		"gl_Position = WorldViewProjection * vec4( pos.xyz, 1 );"
		"}";

	/*GLint BlockModelShader = */this->CreateShader(11, vertexSource11, fragmentSource11);

	this->SetVDSize(11,3);
	this->AddVDElement(11,ELEMENT_FLOAT3,USAGE_POSITION);
	this->AddVDElement(11,ELEMENT_COLOR,USAGE_COLOR);
	this->AddVDElement(11,ELEMENT_FLOAT2,USAGE_NONE);//texcoords not used

	EntityTextureUnif = glGetUniformLocation(this->shaders[11]->program, "texture");
	EntityLightUnif = glGetUniformLocation(this->shaders[11]->program, "light");

	colorUnif2 = glGetUniformLocation(this->shaders[14]->program, "color");
	colorUnif = glGetUniformLocation(this->shaders[13]->program, "color");
	texUnif = glGetUniformLocation( this->shaders[13]->program, "texture");

	glGenBuffers( 1, &rect_vb );
#endif
}

#ifndef USEOPENGL
void CRenderer::Present()
{
	if (this->chain)
	{
		this->chain->Present(this->vsync ? 1 : 0, 0);
	}
}

void CRenderer::Resize(int xres, int yres)
{
	if (xres == 0 || yres == 0)
		return;

	//unbind render targets
	ID3D11RenderTargetView* t = 0;
	context->OMSetRenderTargets(1, &t, 0);

	int left = renderTargetView->Release();
	left = depthStencilView->Release();

	HRESULT result = chain->ResizeBuffers(0, 0/*xres*/, 0/*yres*/, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(result))
		throw 7;

	D3D11_VIEWPORT viewport;
	viewport.Height = yres;
	viewport.Width = xres;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;
	viewport.TopLeftX = viewport.TopLeftY = 0;
	context->RSSetViewports(1, &viewport);

	// Get the pointer to the back buffer.
	ID3D11Resource* backBufferPtr;
	result = chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
		throw 7;

	// Create the render target view with the back buffer pointer.
	//ID3D11RenderTargetView* renderTargetView;
	result = device->CreateRenderTargetView(backBufferPtr, NULL, &renderTargetView);
	if (FAILED(result))
		throw 7;

	// Release pointer to the back buffer as we no longer need it.
	left = backBufferPtr->Release();
	backBufferPtr = 0;

	//create depth buffer
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory((void*)&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = xres;
	depthBufferDesc.Height = yres;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = msaa_count;// 4;// 1;
	depthBufferDesc.SampleDesc.Quality = msaa_quality;// 1;//;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	ID3D11Texture2D* depthStencilBuffer;
	result = device->CreateTexture2D(&depthBufferDesc, 0, &depthStencilBuffer);
	if (FAILED(result))
		throw 7;

	//then create view for depth
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = msaa_count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
	if (FAILED(result))
		throw 7;

	left = depthStencilBuffer->Release();

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	this->xres = xres;
	this->yres = yres;
}
#else
void CRenderer::Present()
{

};
#endif

void CRenderer::Clear(bool cleardepth, bool clearstencil)
{
#ifndef USEOPENGL
	throw 7;
	/*int flags = D3DCLEAR_TARGET;
	if (cleardepth)
	flags |= D3DCLEAR_ZBUFFER;

	if (clearstencil)
	flags |= D3DCLEAR_STENCIL;*/

	if (cleardepth)
		context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//d3ddev->Clear(0, NULL, flags, 0xFFFFFFFF, 1.0f, 0);
#else
	GLbitfield bit = GL_COLOR_BUFFER_BIT;
	if (cleardepth)
		bit |= GL_DEPTH_BUFFER_BIT;

	if (clearstencil)
		bit |= GL_STENCIL_BUFFER_BIT;

	glClear(bit);
#endif
}

void CRenderer::Clear(int color)
{
#ifdef USEOPENGL
	glClearColor(0,0,0,0);//r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	throw 7;
	float colors[4] = { 1.0f, 1.0f, ((float)(color & 0xFF)) / 255.0f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, colors);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
#endif
};

void CRenderer::Clear(float a, float r, float g, float b)
{
#ifdef USEOPENGL
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	float color[4] = { r, g, b, a };
	context->ClearRenderTargetView(renderTargetView, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
#endif
};

void CRenderer::ClearDepth()
{
#ifdef USEOPENGL
	glClear(GL_DEPTH_BUFFER_BIT);
#else
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
#endif
}

void CRenderer::DrawPrimitive(enum PrimitiveType mode, unsigned int offset, unsigned int vertices)
{
	if (vertices == 0)
		return;//uh oh

	this->stats.vertices += vertices;
	this->stats.drawcalls += 1;

#ifdef USEOPENGL
	CShader* sh = this->shaders[this->shaderid];
	if (sh->elements != 0)
	{
		int i = 0; int ei = 0;
		VertexElement* elm = &sh->elements[0];
		while (elm->Type != 8888)//apply VAPs
		{
			int size = 0;
			if (elm->Type == ELEMENT_FLOAT)
				size = 1;
			else if (elm->Type == ELEMENT_FLOAT2)
				size = 2;
			else 
				size = 3;
			if (elm->Usage != USAGE_NONE)
			{
				if (elm->Type == ELEMENT_COLOR)
					glVertexAttribPointer(ei, 4, GL_UNSIGNED_BYTE, GL_TRUE, sh->currentpos, (void*)elm->Offset);
				else
					glVertexAttribPointer(ei, size, GL_FLOAT, GL_FALSE, sh->currentpos, (void*)elm->Offset);
				glEnableVertexAttribArray(ei);
				ei++;
			}
			//char o[200];
			//sprintf(o,"VAP: %i, %i, GL_FLOAT, false, %i, %i",i,size,currentpos,elm->Offset);
			//log(o);

			i++;
			elm = &sh->elements[i];
		}
	}

	glDrawArrays( (int)mode, offset, vertices );
#else
	if (mode != PT_TRIANGLELIST)
		stats.triangles += vertices;
	else
		stats.triangles += vertices / 3;//oh well, not accurate

	this->shader->BindIL(&this->input_layout);

	this->SetPrimitiveType(mode);

	context->Draw(vertices, offset);
#endif
}

void CRenderer::DrawIndexedPrimitive(enum PrimitiveType mode, unsigned int minvertexindex, unsigned int startindex, unsigned int vertices, unsigned int numindices)
{
	if (vertices == 0)
		return;//uh oh

	this->stats.vertices += vertices;
	this->stats.triangles += numindices / 3;
	this->stats.drawcalls += 1;

#ifdef USEOPENGL
	CShader* sh = this->shaders[this->shaderid];
	if (sh->elements != 0)
	{
		int i = 0; int ei = 0;
		VertexElement* elm = &sh->elements[0];
		while (elm->Type != 8888)//apply VAPs
		{
			int size = 0;
			if (elm->Type == ELEMENT_FLOAT)
				size = 1;
			else if (elm->Type == ELEMENT_FLOAT2)
				size = 2;
			else 
				size = 3;
			if (elm->Usage != USAGE_NONE)
			{
				if (elm->Type == ELEMENT_COLOR)
					glVertexAttribPointer(ei, 4, GL_UNSIGNED_BYTE, GL_TRUE, sh->currentpos, (void*)elm->Offset);
				else if (elm->Type == ELEMENT_UBYTE4)
					glVertexAttribPointer(ei, 4, GL_UNSIGNED_BYTE, GL_FALSE, sh->currentpos, (void*)elm->Offset);
				else
					glVertexAttribPointer(ei, size, GL_FLOAT, GL_FALSE, sh->currentpos, (void*)elm->Offset);
				glEnableVertexAttribArray(ei);
				//ei++;

				//char o[200];
				//sprintf(o,"VAP: %i, %i, GL_FLOAT, false, %i, %i",ei,size,sh->currentpos,elm->Offset);
				//log(o);
				ei++;
			}

			i++;
			elm = &sh->elements[i];
		}
	}
	//GLenum e = glGetError();
	//if (e)
	//log("GL ERROR: %d", e);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)0);
	glDrawElements((int)mode, primitives, GL_UNSIGNED_SHORT, (GLvoid*)(startindex*sizeof(unsigned short)));//glDrawArrays( (int)mode, offset, vertices );
#else

	this->shader->BindIL(&this->input_layout);
	//if (mode == PT_TRIANGLELIST)
	//primitives /= 3;

	this->SetPrimitiveType(mode);

	context->DrawIndexed(numindices, minvertexindex, startindex);
	//d3ddev->DrawIndexedPrimitive((D3DPRIMITIVETYPE)mode,0,minvertexindex,vertices,startindex,primitives);
#endif
}

#ifdef _WIN32
void CRenderer::CreateShader(int id, char* vloc, char* vfunc, char* ploc, char* pfunc)
{
	logf("Loading Shader ID %d from file\n", id);

	this->shaders[id] = new CShader(vloc, vfunc, ploc, pfunc);
}
#endif

void CRenderer::CreateShader(int id, const char* vs, const char* ps)
{
	logf("Loading Shader ID %d from string\n", id);

	this->shaders[id] = new CShader(vs, ps);
}


#include "RenderTexture.h"
void CRenderer::SetRenderTarget(int id, const CRenderTexture* rt)
{
	//d3ddev->EndScene();
	context->OMSetRenderTargets(1, &rt->color, rt->depth);
	//d3ddev->SetRenderTarget(id, rt->color);
	//if (id == 0)
	//{
	//d3ddev->SetDepthStencilSurface(rt->depth);
	//}
	//d3ddev->BeginScene();
}

CRenderTexture CRenderer::GetRenderTarget(int id)
{
	CRenderTexture tex;
	context->OMGetRenderTargets(1, &tex.color, &tex.depth);
	tex.color->Release();
	tex.depth->Release();
	return tex;
}

//todo, keep track of matrices when they are set for multiplication later as individual matrices change
//then on draw, multiply the matrices and set the shader constant if any of the matrices have changed
void CRenderer::SetMatrix(int type, const Matrix4* mat)
{
	if (type == WORLD_MATRIX)
	{
		this->world = *mat;
	}
	else if (type == VIEW_MATRIX)
	{
		this->view = *mat;
	}
	else if (type == PROJECTION_MATRIX)
	{
		this->projection = *mat;
	}
	this->_wvpDirty = true;
}

CShader* CRenderer::SetShader(int id)//todo, add caching of last set shader
{
	if (this->shader == this->shaders[id])
		return this->shaders[id];//yay optimization

	this->stats.shader_changes++;

	this->shader = this->shaders[id];
	if (id == 0)
		throw 7;//cant be 0

	if (shader == 0)
		return 0;

	context->VSSetShader(shader->vshader, 0, 0);
	context->PSSetShader(shader->pshader, 0, 0);
	return this->shaders[id];
}

CShader* CRenderer::SetShader(CShader* shdr)//todo, add caching of last set shader
{
	if (this->shader == shdr)
		return shdr;//yay optimization

	this->stats.shader_changes++;

	this->shader = shdr;
	if (shdr == 0)
		throw 7;

	context->VSSetShader(shader->vshader, 0, 0);
	context->PSSetShader(shader->pshader, 0, 0);
	//if (shader->gshader)
	//	context->GSSetShader(shader->gshader, 0, 0);

	return shdr;
}


void CRenderer::DrawRect(Rect* rct, COLOR vertexColor, bool setShader)
{
#ifdef USEOPENGL
	this->DrawRectUV(rct, 0.0f, 1.0f, 0.0f, 1.0f, vertexColor);
#else
	int rhw = 1.0f;

	TLVERTEX vertices[4];

	//Lock the vertex buffer
	//this->rectangle_v_buffer->Lock(0, 0, (void**)&vertices, NULL);

	//Setup vertices
	//A -0.5f modifier is applied to vertex coordinates to match texture
	//and screen coords. Some drivers may compensate for this
	//automatically, but on others texture alignment errors are introduced
	//More information on this can be found in the Direct3D 9 documentation
	vertices[0].color = vertexColor;
	vertices[0].x = (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[0].y = 1.0f - (float)rct->top / (float)(yres / 2);
	vertices[0].z = 0.0f;
	vertices[0].rhw = rhw;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	vertices[1].color = vertexColor;
	vertices[1].x = (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[1].y = 1.0f - (float)rct->top / (float)(yres / 2);
	vertices[1].z = 0.0f;
	vertices[1].rhw = rhw;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	vertices[2].color = vertexColor;
	vertices[2].x = (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[2].y = 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[2].z = 0.0f;
	vertices[2].rhw = rhw;
	vertices[2].u = 0.0f;
	vertices[2].v = 1.0f;

	vertices[3].color = vertexColor;
	vertices[3].x = (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[3].y = 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 1.0f;
	vertices[3].v = 1.0f;

	//this->rectangle_v_buffer.SetVertexDeclaration(this->GetVertexDeclaration(8));
	this->rectangle_v_buffer.Data(vertices, sizeof(vertices), sizeof(TLVERTEX));

	//D3DXMATRIX matIdentity;
	//D3DXMatrixIdentity(&matIdentity);
	//d3ddev->SetTransform(D3DTS_WORLD, &matIdentity);

	this->SetCullmode(CULL_NONE);
	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (setShader)
	{
		if (this->current_texture)
			this->SetShader(16);
		else
			this->SetShader(15);
	}

	this->rectangle_v_buffer.Bind();

	//set the shader here
	//Draw image
	this->DrawPrimitive(PT_TRIANGLEFAN, 0, 4);

	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
#endif
}

void CRenderer::DrawFullScreenQuad()
{
#ifdef USEOPENGL
	this->DrawRectUV(rct, 0.0f, 1.0f, 0.0f, 1.0f, vertexColor);
#else
	int rhw = 1.0f;

	TLVERTEX vertices[4];

	int vertexColor = 0xffffffff;
	//Lock the vertex buffer
	//this->rectangle_v_buffer->Lock(0, 0, (void**)&vertices, NULL);
	//fix this
	//Setup vertices
	//A -0.5f modifier is applied to vertex coordinates to match texture
	//and screen coords. Some drivers may compensate for this
	//automatically, but on others texture alignment errors are introduced
	//More information on this can be found in the Direct3D 9 documentation
	vertices[0].color = vertexColor;
	vertices[0].x = -1;// (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[0].y = 1.0f;// -(float)rct->top / (float)(yres / 2);
	vertices[0].z = 0.0f;
	vertices[0].rhw = rhw;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	vertices[1].color = vertexColor;
	vertices[1].x = 1;// (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[1].y = 1.0f;// -(float)rct->top / (float)(yres / 2);
	vertices[1].z = 0.0f;
	vertices[1].rhw = rhw;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	vertices[2].color = vertexColor;
	vertices[2].x = -1;// (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[2].y = -1.0f;// -(float)rct->bottom / (float)(yres / 2);
	vertices[2].z = 0.0f;
	vertices[2].rhw = rhw;
	vertices[2].u = 0.0f;
	vertices[2].v = 1.0f;

	vertices[3].color = vertexColor;
	vertices[3].x = 1;// (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[3].y = -1;// 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 1.0f;
	vertices[3].v = 1.0f;

	//this->rectangle_v_buffer.SetVertexDeclaration(this->GetVertexDeclaration(8));
	this->rectangle_v_buffer.Data(vertices, sizeof(vertices), sizeof(TLVERTEX));

	//D3DXMATRIX matIdentity;
	//D3DXMatrixIdentity(&matIdentity);
	//d3ddev->SetTransform(D3DTS_WORLD, &matIdentity);

	this->SetCullmode(CULL_NONE);
	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	/*if (setShader)
	{
	if (this->current_texture)
	this->SetShader(16);
	else
	this->SetShader(15);
	}*/

	this->rectangle_v_buffer.Bind();

	//set the shader here
	//Draw image
	this->DrawPrimitive(PT_TRIANGLEFAN, 0, 4);

	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
#endif
}

//ok, give each shader multiple input topologys, then each vb indexes into
//the array to get the right one to use

void CRenderer::DrawRectUV(Rect* rct, float minu, float maxu, float minv, float maxv, COLOR color)
{
#ifndef USEOPENGL
	//add a shader for rects
	TLVERTEX vertices[4];

	//Lock the vertex buffer
	//this->rectangle_v_buffer->Lock(0, 0, (void**)&vertices, NULL);

	//Setup vertices
	//A -0.5f modifier is applied to vertex coordinates to match texture
	//and screen coords. Some drivers may compensate for this
	//automatically, but on others texture alignment errors are introduced
	//More information on this can be found in the Direct3D 9 documentation
	vertices[0].color = color;
	vertices[0].x = (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[0].y = 1.0f - (float)rct->top / (float)(yres / 2);
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].color = color;
	vertices[1].x = (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[1].y = 1.0f - (float)rct->top / (float)(yres / 2);
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = maxu;
	vertices[1].v = minv;

	vertices[2].color = color;
	vertices[2].x = (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[2].y = 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = minu;
	vertices[2].v = maxv;

	vertices[3].color = color;
	vertices[3].x = (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[3].y = 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = maxu;
	vertices[3].v = maxv;

	//Handle rotation
	/*if (rotate != 0)
	{
	RECT rOrigin;
	float centerX, centerY;

	//Find center of destination rectangle
	centerX = (float)(rDest->left + rDest->right) / 2;
	centerY = (float)(rDest->top + rDest->bottom) / 2;

	//Translate destination rect to be centered on the origin
	rOrigin.top = rDest->top - (int)(centerY);
	rOrigin.bottom = rDest->bottom - (int)(centerY);
	rOrigin.left = rDest->left - (int)(centerX);
	rOrigin.right = rDest->right - (int)(centerX);

	//Rotate vertices about the origin
	bufferVertices[index].x = rOrigin.left * cosf(rotate) -
	rOrigin.top * sinf(rotate);
	bufferVertices[index].y = rOrigin.left * sinf(rotate) +
	rOrigin.top * cosf(rotate);

	bufferVertices[index + 1].x = rOrigin.right * cosf(rotate) -
	rOrigin.top * sinf(rotate);
	bufferVertices[index + 1].y = rOrigin.right * sinf(rotate) +
	rOrigin.top * cosf(rotate);

	bufferVertices[index + 2].x = rOrigin.right * cosf(rotate) -
	rOrigin.bottom * sinf(rotate);
	bufferVertices[index + 2].y = rOrigin.right * sinf(rotate) +
	rOrigin.bottom * cosf(rotate);

	bufferVertices[index + 3].x = rOrigin.left * cosf(rotate) -
	rOrigin.bottom * sinf(rotate);
	bufferVertices[index + 3].y = rOrigin.left * sinf(rotate) +
	rOrigin.bottom * cosf(rotate);

	//Translate vertices to proper position
	bufferVertices[index].x += centerX;
	bufferVertices[index].y += centerY;
	bufferVertices[index + 1].x += centerX;
	bufferVertices[index + 1].y += centerY;
	bufferVertices[index + 2].x += centerX;
	bufferVertices[index + 2].y += centerY;
	bufferVertices[index + 3].x += centerX;
	bufferVertices[index + 3].y += centerY;
	}*/

	this->rectangle_v_buffer.Data(vertices, sizeof(TLVERTEX) * 4, sizeof(TLVERTEX));

	//Unlock the vertex buffer
	//this->rectangle_v_buffer->Unlock();

	//D3DXMATRIX matIdentity;
	//D3DXMatrixIdentity(&matIdentity);
	//d3ddev->SetTransform(D3DTS_WORLD, &matIdentity);

	this->SetCullmode(CULL_NONE);
	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (this->current_texture == 0)
		this->SetShader(15);
	else
		this->SetShader(16);

	//this->rectangle_v_buffer.SetVertexDeclaration(this->GetVertexDeclaration(8));
	this->rectangle_v_buffer.Bind();

	//Draw image
	//this->d3ddev->SetFVF(D3DFVF_TLVERTEX);
	//this->d3ddev->SetStreamSource(0, this->rectangle_v_buffer, 0, sizeof(TLVERTEX));
	this->DrawPrimitive(PT_TRIANGLEFAN, 0, 4);

	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
#else
	if (this->current_texture != 0)
	{
		this->SetShader(13);

		glBindBuffer( GL_ARRAY_BUFFER, this->rect_vb);

		//position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*4, 0);

		//glUniform4f(colorUnif, 1.0f, 1.0f, 1.0f, 0.2f);
		glUniform4f(colorUnif, ((float)((color & 0x00FF0000) >> 16))/255.0f, ((float)((color & 0x0000FF00) >> 8))/255.0f,((float)(color & 0x000000FF))/255.0f,(float)((color & 0xFF000000 >> 24))/255.0f);
		glUniform1i(texUnif, 0);

		float sx = 2.0f / (float)xres;
		float sy = 2.0f / (float)yres;

		float x = rct->left*sx - 1.0f;// + g->bitmap_left * sx;
		float y = -rct->top*sy + 1.0f;//y*sy + 1.0f;// - g->bitmap_top * sy;
		float w = rct->right - rct->left;//this->width * sx;
		float h = rct->bottom - rct->top;//this->height * sy;
		w *= sx;
		h *= sy;

		GLfloat box[4][4] = {
			{x,     y    , minu, minv},//0,0
			{x + w, y    , maxu, minv},//1,0
			{x,     y - h, minu, maxv},//0,1
			{x + w, y - h, maxu, maxv},//1,1
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
	}
	else
	{
		this->SetShader(14);

		glBindBuffer( GL_ARRAY_BUFFER, this->rect_vb);

		//position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);

		glUniform4f(colorUnif2, ((float)((color & 0x00FF0000) >> 16))/255.0f, ((float)((color & 0x0000FF00) >> 8))/255.0f,((float)(color & 0x000000FF))/255.0f,(float)((color & 0xFF000000 >> 24))/255.0f);
		//glUniform4f(colorUnif2, 1.0f, 1.0f, 1.0f, 0.2f);
		//glUniform1i(texUnif, 0);

		float sx = 2.0f / (float)xres;
		float sy = 2.0f / (float)yres;

		float x = rct->left*sx - 1.0f;// + g->bitmap_left * sx;
		float y = -rct->top*sy + 1.0f;//y*sy + 1.0f;// - g->bitmap_top * sy;
		float w = rct->right - rct->left;//this->width * sx;
		float h = rct->bottom - rct->top;//this->height * sy;
		w *= sx;
		h *= sy;

		GLfloat box[4][2] = {
			{x,     y    },//0,0
			{x + w, y    },//1,0
			{x,     y - h},//0,1
			{x + w, y - h},//1,1
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindBuffer( GL_ARRAY_BUFFER, 0);
#endif
}


void CRenderer::DrawRectUV(Rect* rct, Vec2 top_left, Vec2 top_right, Vec2 bottom_left, Vec2 bottom_right, COLOR color)
{
#ifndef USEOPENGL
	//add a shader for rects
	TLVERTEX vertices[4];

	//Lock the vertex buffer
	//this->rectangle_v_buffer->Lock(0, 0, (void**)&vertices, NULL);

	//Setup vertices
	//A -0.5f modifier is applied to vertex coordinates to match texture
	//and screen coords. Some drivers may compensate for this
	//automatically, but on others texture alignment errors are introduced
	//More information on this can be found in the Direct3D 9 documentation
	vertices[0].color = color;
	vertices[0].x = (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[0].y = 1.0f - (float)rct->top / (float)(yres / 2);
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = top_left.x;
	vertices[0].v = top_left.y;

	vertices[1].color = color;
	vertices[1].x = (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[1].y = 1.0f - (float)rct->top / (float)(yres / 2);
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = top_right.x;
	vertices[1].v = top_right.y;

	vertices[2].color = color;
	vertices[2].x = (float)rct->left / (float)(xres / 2) - 1.0f;
	vertices[2].y = 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = bottom_left.x;// minu;
	vertices[2].v = bottom_left.y;// maxv;

	vertices[3].color = color;
	vertices[3].x = (float)rct->right / (float)(xres / 2) - 1.0f;
	vertices[3].y = 1.0f - (float)rct->bottom / (float)(yres / 2);
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = bottom_right.x;// maxu;
	vertices[3].v = bottom_right.y;// maxv;

	//Handle rotation
	/*if (rotate != 0)
	{
	RECT rOrigin;
	float centerX, centerY;

	//Find center of destination rectangle
	centerX = (float)(rDest->left + rDest->right) / 2;
	centerY = (float)(rDest->top + rDest->bottom) / 2;

	//Translate destination rect to be centered on the origin
	rOrigin.top = rDest->top - (int)(centerY);
	rOrigin.bottom = rDest->bottom - (int)(centerY);
	rOrigin.left = rDest->left - (int)(centerX);
	rOrigin.right = rDest->right - (int)(centerX);

	//Rotate vertices about the origin
	bufferVertices[index].x = rOrigin.left * cosf(rotate) -
	rOrigin.top * sinf(rotate);
	bufferVertices[index].y = rOrigin.left * sinf(rotate) +
	rOrigin.top * cosf(rotate);

	bufferVertices[index + 1].x = rOrigin.right * cosf(rotate) -
	rOrigin.top * sinf(rotate);
	bufferVertices[index + 1].y = rOrigin.right * sinf(rotate) +
	rOrigin.top * cosf(rotate);

	bufferVertices[index + 2].x = rOrigin.right * cosf(rotate) -
	rOrigin.bottom * sinf(rotate);
	bufferVertices[index + 2].y = rOrigin.right * sinf(rotate) +
	rOrigin.bottom * cosf(rotate);

	bufferVertices[index + 3].x = rOrigin.left * cosf(rotate) -
	rOrigin.bottom * sinf(rotate);
	bufferVertices[index + 3].y = rOrigin.left * sinf(rotate) +
	rOrigin.bottom * cosf(rotate);

	//Translate vertices to proper position
	bufferVertices[index].x += centerX;
	bufferVertices[index].y += centerY;
	bufferVertices[index + 1].x += centerX;
	bufferVertices[index + 1].y += centerY;
	bufferVertices[index + 2].x += centerX;
	bufferVertices[index + 2].y += centerY;
	bufferVertices[index + 3].x += centerX;
	bufferVertices[index + 3].y += centerY;
	}*/

	this->rectangle_v_buffer.Data(vertices, sizeof(TLVERTEX) * 4, sizeof(TLVERTEX));

	//Unlock the vertex buffer
	//this->rectangle_v_buffer->Unlock();

	//D3DXMATRIX matIdentity;
	//D3DXMatrixIdentity(&matIdentity);
	//d3ddev->SetTransform(D3DTS_WORLD, &matIdentity);

	this->SetCullmode(CULL_NONE);
	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (this->current_texture == 0)
		this->SetShader(15);
	else
		this->SetShader(16);

	this->rectangle_v_buffer.Bind();

	//Draw image
	//this->d3ddev->SetFVF(D3DFVF_TLVERTEX);
	//this->d3ddev->SetStreamSource(0, this->rectangle_v_buffer, 0, sizeof(TLVERTEX));
	this->DrawPrimitive(PT_TRIANGLEFAN, 0, 4);

	//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
#else
	if (this->current_texture != 0)
	{
		this->SetShader(13);

		glBindBuffer(GL_ARRAY_BUFFER, this->rect_vb);

		//position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * 4, 0);

		//glUniform4f(colorUnif, 1.0f, 1.0f, 1.0f, 0.2f);
		glUniform4f(colorUnif, ((float)((color & 0x00FF0000) >> 16)) / 255.0f, ((float)((color & 0x0000FF00) >> 8)) / 255.0f, ((float)(color & 0x000000FF)) / 255.0f, (float)((color & 0xFF000000 >> 24)) / 255.0f);
		glUniform1i(texUnif, 0);

		float sx = 2.0f / (float)xres;
		float sy = 2.0f / (float)yres;

		float x = rct->left*sx - 1.0f;// + g->bitmap_left * sx;
		float y = -rct->top*sy + 1.0f;//y*sy + 1.0f;// - g->bitmap_top * sy;
		float w = rct->right - rct->left;//this->width * sx;
		float h = rct->bottom - rct->top;//this->height * sy;
		w *= sx;
		h *= sy;

		GLfloat box[4][4] = {
			{ x, y, minu, minv },//0,0
			{ x + w, y, maxu, minv },//1,0
			{ x, y - h, minu, maxv },//0,1
			{ x + w, y - h, maxu, maxv },//1,1
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
	}
	else
	{
		this->SetShader(14);

		glBindBuffer(GL_ARRAY_BUFFER, this->rect_vb);

		//position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);

		glUniform4f(colorUnif2, ((float)((color & 0x00FF0000) >> 16)) / 255.0f, ((float)((color & 0x0000FF00) >> 8)) / 255.0f, ((float)(color & 0x000000FF)) / 255.0f, (float)((color & 0xFF000000 >> 24)) / 255.0f);
		//glUniform4f(colorUnif2, 1.0f, 1.0f, 1.0f, 0.2f);
		//glUniform1i(texUnif, 0);

		float sx = 2.0f / (float)xres;
		float sy = 2.0f / (float)yres;

		float x = rct->left*sx - 1.0f;// + g->bitmap_left * sx;
		float y = -rct->top*sy + 1.0f;//y*sy + 1.0f;// - g->bitmap_top * sy;
		float w = rct->right - rct->left;//this->width * sx;
		float h = rct->bottom - rct->top;//this->height * sy;
		w *= sx;
		h *= sy;

		GLfloat box[4][2] = {
			{ x, y },//0,0
			{ x + w, y },//1,0
			{ x, y - h },//0,1
			{ x + w, y - h },//1,1
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

void CRenderer::AddPoint(float value)
{
	pos = (++pos % 200);
	this->values[pos].value = value;
	this->values[pos].text = false;
}

void CRenderer::DrawGraph()
{
	this->SetFont("airal", 12);
	float h = 100;
	Rect r;
	r.left = this->xres - 200 * 2 - 10;
	r.right = this->xres - 199 * 2 - 10;
	r.bottom = this->yres - 75;

	float total = 0;
	float count = 0;
	for (int i = 0; i < 200; i++)
	{
		if (values[i].value > 0.01f)
		{
			total += values[i].value;
			count++;
		}
	}
	float average = 0;
	if (count > 0)
		average = total / count;

	int difsum = 0;
	for (int i = 0; i < 200; i++)
	{
		if (values[i].value > 0.01f)
		{
			float dif = values[i].value - average;
			difsum += dif*dif;
		}
	}
	float stdv = difsum ? sqrt(difsum / count) : 0;

	float scale = 0.25f;
	if (average > 400)
		scale = 0.125f;

	int since = 0;
	for (int i = pos + 1; i < 200; i++)
	{
		r.top = r.bottom - this->values[i].value*scale;

		this->DrawRect(&r, 0xFFFF0000);
		if ((values[i].value > average + stdv && values[i].value > 0.01f && since-- <= 0) || this->values[i].text == true)
		{
			char o[10];
			sprintf(o, "%0.f", this->values[i].value);
			this->DrawText(r.left, r.top - 25, o, 0xFFFFFFFF);
			this->values[i].text = true;
			since = 5;
		}
		r.left += 2;
		r.right += 2;
	}
	since = 0;
	for (int i = 0; i < pos; i++)
	{
		r.top = r.bottom - this->values[i].value*scale;

		this->DrawRect(&r, 0xFFFF0000);
		if ((values[i].value > average + stdv && values[i].value > 0.01f && since-- <= 0) || this->values[i].text == true)
		{
			char o[10];
			sprintf(o, "%0.f", this->values[i].value);
			this->DrawText(r.left, r.top - 25, o, 0xFFFFFFFF);
			this->values[i].text = true;
			since = 5;
		}
		r.left += 2;
		r.right += 2;
	}
	this->SetFont("Arial", 20);
}

void CRenderer::DrawText(int x, int y, const char* text, COLOR color)
{
	float sx = 2.0f / (float)xres;
	float sy = 2.0f / (float)yres;

	this->font->DrawText(text, x*sx - 1.0f, y*sy, this->fontsize, this->fontsize, color);
}

Rect CRenderer::DrawText(Rect r, const char* text, COLOR color, int flags)
{
	int dx = r.Width();//r.right - r.left;
	int dy = r.Height();//r.bottom - r.top;
	//ok, lets insert line breaks at correct spots
	int s = this->font->TextSize(text);
	char* cpy = 0;
	Rect si(r.top, r.top + this->fontsize + 1, r.left, r.left + s);
	//si.left = r.left;
	//si.right = r.left + s;
	//si.top = r.top;
	//si.bottom = r.top + this->fontsize + 1;
	if (s > dx)
	{
		if (dx < 0)
			dx = 30;

		cpy = new char[strlen(text) + 1 + (s / dx + 1)];
		strcpy(cpy, text);

		//break it up
		int cur = 0;//line cursor
		char* line = cpy;
		while (this->font->TextSize(line) > dx)
		{
			//find where to break
			bool broken = false;
			for (int i = strlen(line); i >= 0; i--)//try doing it at spaces first
			{
				if (line[i] == ' ' && this->font->TextSize(line, i) < dx)
				{
					line[i] = '\n';
					si.bottom += this->fontsize + 1;
					broken = true;
					line += i + 1;
					break;
				}
			}
			if (broken == false)
			{
				//just break at last possible point
				int p = strlen(line);
				while (this->font->TextSize(line, p) > dx && p > 0)
				{
					p--;
				}
				if (p != 0)
				{
					//insert line break
					si.bottom += this->fontsize + 1;
					char tmp = line[p];
					line[p] = '\n';//todo actually do insert later

					int i = p;//strlen(line);
					int max = strlen(line) + 1;
					while (i++ < max)
					{
						char tmp2 = line[i];
						line[i] = tmp;
						tmp = tmp2;
					}

					line += p + 1;//cur += p+1;
				}
				else
				{
					//not wide enough
					throw 7;
				}
			}
		}
		s = dx;
		si.right = r.right;
	}

	//0 aligns top
	if (flags == 1)// & ALIGN_BOTTOM)
	{
		int dy = si.Height();//si.bottom - si.top;
		si.bottom = r.bottom;
		si.top = si.bottom - dy;
	}
	else if (flags == 2)//center vertically
	{
		int dy = si.Height();//si.bottom - si.top;
		int rdy = r.bottom - r.top;

		int left = rdy - dy;
		//if left < 0 not enough room

		si.bottom = si.bottom - left / 2;
		si.top = si.top + left / 2;
	}
	if (flags & ALIGN_RIGHT)
	{
		si.left = r.right - s;
	}

	if (color != 0)
	{
		float sx = 2.0f / (float)xres;
		float sy = 2.0f / (float)yres;
		this->font->DrawText(cpy ? cpy : text, si.left*sx - 1.0f, si.top*sy, this->fontsize, this->fontsize, color);
	}

	if (cpy)
		delete[] cpy;

	return si;
}

void CRenderer::DrawVerticalCenteredText(Rect r, const char* text, COLOR color)
{
	float x = r.left;// + ((r.right - r.left - font->TextSize(text))/2.0f);
	float y = r.top + ((r.Height() - this->fontsize) / 2.0f);
	float sx = 2.0f / (float)xres;
	float sy = 2.0f / (float)yres;

	this->font->DrawText(text, x*sx - 1.0f, y*sy, this->fontsize, this->fontsize, color);
}
#undef DrawText
void CRenderer::DrawCenteredText(Rect r, const char* text, COLOR color)
{
	float x = r.left + ((r.Width() - this->font->TextSize(text)) / 2.0f);
	float y = r.top + ((r.Height() - this->fontsize) / 2.0f);
	float sx = 2.0f / (float)xres;
	float sy = 2.0f / (float)yres;

	this->font->DrawText(text, x*sx - 1.0f, y*sy, this->fontsize, this->fontsize, color);
}

void CRenderer::ClipToScreenPosition(float &x, float &y)
{
	float sx = 2.0f / (float)xres;
	float sy = 2.0f / (float)yres;
	sx = 1.0f / sx;
	sy = 1.0f / sy;

	x += 1.0f;
	y += 1.0f;

	x *= sx;
	y *= sy;
}

void CRenderer::ScreenToClipPosition(float &x, float &y)
{
	float sx = 2.0f / (float)xres;
	float sy = 2.0f / (float)yres;

	x *= sx;
	y *= sy;

	x -= 1.0f;
	y -= 1.0f;
}


void CRenderer::DrawStats(float frametime, float realframetime, unsigned int memuse, float rp)
{
	this->SetFont("arial", 12);
	this->SetPixelTexture(0, (Texture)0);
	this->EnableAlphaBlending(true);

	Rect r;
	r.top = 70;
	r.bottom = 70 + (8 * 14);
	r.left = this->xres - 220;
	r.right = this->xres;
	this->DrawRect(&r, COLOR_ARGB(200, 100, 100, 100));

	char text[2000];
	std::string memstr = FormatMemory(memuse);
	sprintf(text, "Time: %.1fms : %0.1ffps\nShader Changes: %d\nBatchCount: %d\nVertices: %d\nTris: %d\nVBOs: %d - %s\nTexs: %d - %s\nMem: %s\nProcess: %.1fms", realframetime*1000.0f, 1 / frametime, this->stats.shader_changes, this->stats.drawcalls, this->stats.vertices, this->stats.triangles, this->stats.vertexbuffers, FormatMemory(this->stats.vertexbuffer_mem).c_str(), this->stats.textures, FormatMemory(this->stats.texture_mem).c_str(), memstr.c_str(), rp*1000.0f);
	this->DrawText(this->xres - 218, 70, text, COLOR_ARGB(255, 255, 255, 255));
	this->EnableAlphaBlending(false);

	this->SetFont("arial", 20);
}

#include "../camera.h"
#include "Renderable.h"
bool CRenderer::WorldToScreen(CCamera* cam, const Vec3 pos, Vec3& out, Parent* parent)
{
	Matrix4f worl;
	Viewport viewport;
	worl.MakeIdentity();
	cam->applyCam();
	this->GetViewport(&viewport);
	if (cam != 0)
	{
		Vec3 in;
		if (cam->parent == 0 && parent)
			in = parent->LocalToWorld(pos);//invmat*Vec3(ply->position.x, ply->position.y + 1.0f, ply->position.z);
		else if (cam->parent == parent)
			in = pos;
		else if (cam->parent && parent)
		{
			in = parent->LocalToWorld(pos);//invmat*Vec3(ply->position.x, ply->position.y + 1.0f, ply->position.z);
			in = cam->parent->WorldToLocal(in);//cam->parent->mat*in;
		}
		else if (cam->parent)
		{
			in = cam->parent->WorldToLocal(pos);
		}

		if (cam->SphereInFrustum(&in, 1.0f))
		{
			out = in.toscreen(&in, &viewport, &projection, &view, &worl);

			return true;
		}

		//not on screen
		return false;
	}
}

void CRenderer::DrawIcon(int x, int y, int size, int id, COLOR color)
{
	float minx = (float)(id % 4) / 4.0f;
	float maxu = (float)(id % 4 + 1) / 4.0f;
	float minv = (float)(id / 4) / 4.0f;
	float maxv = (float)(id / 4 + 1) / 4.0f;//1.0f/4.0f;

	auto tex = resources.get<CTexture>("icons.png");
	this->SetPixelTexture(0, tex->texture);
	this->EnableAlphaBlending(true);
	Rect r;
	r.top = y - size / 2;
	r.bottom = y + size / 2;
	r.left = x - size / 2;
	r.right = x + size / 2;
	this->DrawRectUV(&r, minx, maxu, minv, maxv, color);
	this->EnableAlphaBlending(false);
}

void CRenderer::DrawNormals(Vec3 pos, Vec3 x, Vec3 y, Vec3 z)
{
	this->SetDepthRange(0.0f, 0.0f);

	struct vertz
	{
		Vec3 p;
		unsigned int color;
	};

	//draw bones
	//Texture old = this->current_texture;
	//this->SetTexture(0,(Texture)0);
	//auto olds = renderer->shader;
	//this->SetShader(0);
	//this->d3ddev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	//Vec3 size = t.joints[i].bb.Size()/2;
	//Vec3 m = jointMats[i].transform(Vec3(0,0,0));
	//OBB b = OBB(t.joints[i].bb, (Matrix4)this->OutFrames[i]);

	//bool picked = SphereIntersect(pos, dir, renderer->world*b.pos, b.r.length());
	//if (picked == false)
	//renderer->DrawBoundingBox(b);
	//else
	//renderer->DrawBoundingBox(b, COLOR_ARGB(255,255,0,0));

	vertz p[6];

	//draw normals
	p[0].p = pos;
	p[0].color = COLOR_ARGB(255, 255, 0, 0);
	p[1].p = pos + (x)*0.375f;
	p[1].color = COLOR_ARGB(255, 255, 0, 0);
	p[2].p = pos;
	p[2].color = COLOR_ARGB(255, 0, 255, 0);
	p[3].p = pos + (y)*0.375f;
	p[3].color = COLOR_ARGB(255, 0, 255, 0);
	p[4].p = pos;
	p[4].color = COLOR_ARGB(255, 0, 0, 255);
	p[5].p = pos + (z)*0.375f;
	p[5].color = COLOR_ARGB(255, 0, 0, 255);

	CVertexBuffer vb;
	vb.Data(p, sizeof(p[0]) * 6, sizeof(p[0]));
	VertexElement elm2[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR } };

	vb.SetVertexDeclaration(this->GetVertexDeclaration(elm2, 2));
	this->SetShader(passthrough);
	vb.Bind();

	//set constant buffer
	auto mat = (Matrix4::Identity()*this->view*this->projection).Transpose();
	passthrough->buffers.wvp.UploadAndSet(&mat, sizeof(mat));

	//VertexElement elm2[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	//{ ELEMENT_COLOR, USAGE_COLOR } };
	//vb.SetVertexDeclaration(this->GetVertexDeclaration(elm2, 2));

	this->shader->BindIL(&vb.vd);
	this->current_pt = PT_LINES;
	this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	this->context->Draw(6, 0);
	//this->d3ddev->DrawPrimitiveUP(D3DPT_LINELIST, 3, &p, sizeof(vertz));

	//this->d3ddev->SetVertexShader(shader->shader);
	//this->d3ddev->SetPixelShader(shader->pshader);
	//renderer->SetShader(olds);
	this->SetDepthRange(0.0f, 1.0f);
}


void CRenderer::DrawFrustum(const Matrix4& view, const Matrix4& proj, COLOR color)
{
	/*const unsigned short index[] = {0, 1,
	1, 2,
	2, 3,
	3, 0,
	4, 5,
	5, 6,
	6, 7,
	7, 4,
	2, 7,//3, 7,//nope
	1, 6,//2, 6,//nope
	3, 4,//0, 4,//nope
	0, 5};//1, 5};//nope

	Matrix4 matrix = view;

	float width = 2.0/proj.m_afEntry[0];
	float hight = 2.0/proj.m_afEntry[5];

	//if (proj.m_afEntry[15] == 0)
	//throw 7;

	float znear = -proj.m_afEntry[11]/proj.m_afEntry[10];//14/10
	float zfar = 1.0f/proj.m_afEntry[10] + znear;

	float a = proj.m_afEntry[5];
	float b = proj.m_afEntry[7];
	float top = (1.0f-b)/a;//(1.0f/a - 1.0f)*b;
	float bottom = -(b+1.0)/a;//-2.0f*b-top;
	if (b <= 0.00001f && b >= -0.00001f)
	{
	top = 1.0f/a;
	bottom = -top;
	}

	a = proj.m_afEntry[0];
	b = proj.m_afEntry[3];
	float right = (1.0f-b)/a;//(1.0f/a - 1.0f)*b;
	float left = -(b+1.0f)/a;//-2.0f*b-right;
	if (b <= 0.00001f && b >= -0.00001f)
	{
	right = 1.0f/a;
	left = -right;
	}

	Matrix4 mat = (proj).Inverse();

	Vec4 cubeCorners[8];
	if (proj.m_afEntry[15] == 0)
	{
	cubeCorners[0] = mat*Vec4(1,-1,0,1);//this->_pos+right*xnear+this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z);
	cubeCorners[1] = mat*Vec4(1,1,0,1);//this->_pos+right*xnear-this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x, ii->_position.y+16, ii->_position.z);
	cubeCorners[2] = mat*Vec4(-1,1,0,1);//this->_pos+right*xfar+this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	cubeCorners[3] = mat*Vec4(-1,-1,0,1);//this->_pos-right*xnear-this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x+16, ii->_position.y, ii->_position.z+16);
	cubeCorners[4] = mat*Vec4(1,-1,1,1);//this->_pos-this->_right*xnear+this->_upDir*ynear+this->_lookAt*this->_near;//Vec3(ii->_position.x+16, ii->_position.y+16, ii->_position.z+16);
	cubeCorners[5] = mat*Vec4(1,1,1,1);//this->_pos+this->_right*xfar-this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	cubeCorners[6] = mat*Vec4(-1,1,1,1);//this->_pos-this->_right*xfar+this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	cubeCorners[7] = mat*Vec4(-1,-1,1,1);//this->_pos-this->_right*xfar-this->_upDir*yfar+this->_lookAt*this->_far;//Vec3(ii->_position.x, ii->_position.y, ii->_position.z+16);
	}
	else
	{
	cubeCorners[0] = Vec4(right, bottom, znear, 1);
	cubeCorners[1] = Vec4(right, top, znear, 1);
	cubeCorners[2] = Vec4(left, top, znear, 1);
	cubeCorners[3] = Vec4(left, bottom, znear, 1);
	cubeCorners[4] = Vec4(right, bottom, zfar, 1);
	cubeCorners[5] = Vec4(right, top, zfar, 1);
	cubeCorners[6] = Vec4(left, top, zfar, 1);
	cubeCorners[7] = Vec4(left, bottom, zfar, 1);
	}
	for (int i = 0; i < 8; i++)
	{
	cubeCorners[i] = (view.Inverse())*cubeCorners[i];
	//only do this if perspective matrix
	if (proj.m_afEntry[15] == 0)
	cubeCorners[i] /= cubeCorners[i].w;
	}

	//CCamera cam;
	//cam._matrix = view;//.Transpose();
	//cam._projectionMatrix = proj;
	//cam.BuildViewFrustum();
	//cam.GetCorners(cubeCorners);
	//keep fixing camera class to return these points
	struct vert
	{
	Vec3 p;
	COLOR c;
	vert(Vec4 p, COLOR c)
	{
	this->p = (Vec3)p;
	this->c = c;
	}
	};
	this->EnableAlphaBlending(true);
	//COLOR color = 0xFFFF0000;
	vert corners[] =
	{
	// face 1
	vert(cubeCorners[6],color), vert(cubeCorners[2],color), vert(cubeCorners[1],color),
	vert(cubeCorners[1],color), vert(cubeCorners[5],color), vert(cubeCorners[6],color),

	// face 2
	vert(cubeCorners[3],color), vert(cubeCorners[2],color), vert(cubeCorners[6],color),
	vert(cubeCorners[6],color), vert(cubeCorners[7],color), vert(cubeCorners[3],color),

	// face 3
	vert(cubeCorners[0],color), vert(cubeCorners[3],color), vert(cubeCorners[7],color),
	vert(cubeCorners[7],color), vert(cubeCorners[4],color), vert(cubeCorners[0],color),

	// face 4
	vert(cubeCorners[5],color), vert(cubeCorners[1],color), vert(cubeCorners[0],color),
	vert(cubeCorners[0],color), vert(cubeCorners[4],color), vert(cubeCorners[5],color),

	// face 5
	vert(cubeCorners[6],color), vert(cubeCorners[5],color), vert(cubeCorners[4],color),
	vert(cubeCorners[4],color), vert(cubeCorners[7],color), vert(cubeCorners[6],color),

	// face 6
	vert(cubeCorners[0],color), vert(cubeCorners[1],color), vert(cubeCorners[2],color),
	vert(cubeCorners[2],color), vert(cubeCorners[3],color), vert(cubeCorners[0],color),
	};

	//renderer->DrawBoundingBox(cubeCorners[0], cubeCorners[7]);
	Matrix4 null = Matrix4::Identity();
	d3ddev->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&null);
	this->SetCullmode(CULL_NONE);
	this->SetDepthRange(0,0);
	this->SetTexture(0,0);
	auto old = this->shader;
	this->SetShader(0);
	this->EnableAlphaBlending(true);
	d3ddev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, corners, sizeof(vert));//IndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 24, 12, (void *)index, D3DFMT_INDEX16, ObjectBound, sizeof(D3DXVECTOR3));
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	this->EnableAlphaBlending(false);
	d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 12, corners, sizeof(vert));//IndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 24, 12, (void *)index, D3DFMT_INDEX16, ObjectBound, sizeof(D3DXVECTOR3));
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	this->SetShader(old);
	this->SetDepthRange(0,1);
	this->SetCullmode(CULL_CW);*/
}

void CRenderer::DrawBoundingBox(const OBB bb, COLOR color)//Vec3* ObjectBound)//needs vector of all 4 sides
{
	const unsigned short index[] = { 0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		2, 7,//3, 7,//nope
		1, 6,//2, 6,//nope
		3, 4,//0, 4,//nope
		0, 5 };//1, 5};//nope

	Vec3 ax = bb.axes[0];
	Vec3 ay = bb.axes[1];
	Vec3 az = bb.axes[2];
	Vec3 p = bb.pos;

	struct vert
	{
		Vec3 p;
		COLOR c;
	};
	vert ObjectBound[8];
	ObjectBound[0].p = p - ax*bb.r.x - ay*bb.r.y - az*bb.r.z;//min;						 //000
	ObjectBound[1].p = p + ax*bb.r.x - ay*bb.r.y - az*bb.r.z;//Vec3(max.x, min.y, min.z);//100
	ObjectBound[2].p = p + ax*bb.r.x + ay*bb.r.y - az*bb.r.z;//Vec3(max.x, max.y, min.z);//110
	ObjectBound[3].p = p - ax*bb.r.x + ay*bb.r.y - az*bb.r.z;//Vec3(min.x, max.y, min.z);//010
	ObjectBound[4].p = p - ax*bb.r.x + ay*bb.r.y + az*bb.r.z;//Vec3(min.x, max.y, max.z);//011
	ObjectBound[5].p = p - ax*bb.r.x - ay*bb.r.y + az*bb.r.z;//Vec3(min.x, min.y, max.z);//001
	ObjectBound[6].p = p + ax*bb.r.x - ay*bb.r.y + az*bb.r.z;//Vec3(max.x, min.y, max.z);//101
	ObjectBound[7].p = p + ax*bb.r.x + ay*bb.r.y + az*bb.r.z;//max;						 //111

	for (int i = 0; i < 8; i++)
		ObjectBound[i].c = color;
	//this->SetCullmode(CULL_NONE);
	/*this->SetDepthRange(0,0);
	auto old = this->shader;
	this->SetShader(0);
	d3ddev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	d3ddev->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 24, 12, (void *)index, D3DFMT_INDEX16, ObjectBound, sizeof(vert));
	this->SetShader(old);
	this->SetDepthRange(0,1);*/

	//auto old = this->shader;
	this->SetShader(this->passthrough);

	CIndexBuffer ib;
	CVertexBuffer vb;
	vb.Data(ObjectBound, 16 * 8, 16);
	ib.Data((void*)index, 12 * 2 * 2, 0);

	VertexElement elm2[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR } };

	vb.SetVertexDeclaration(this->GetVertexDeclaration(elm2,2));

	this->shader->BindIL(&vb.vd);

	vb.Bind();
	ib.Bind();

	//set constant buffer
	auto mat = (Matrix4::Identity()*this->view*this->projection).Transpose();
	passthrough->buffers.wvp.UploadAndSet(&mat, sizeof(mat));
	//VertexElement elm2[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	//{ ELEMENT_COLOR, USAGE_COLOR } };

	//vb.SetVertexDeclaration(this->GetVertexDeclaration(elm2,2));


	//this->SetCullmode(CULL_NONE);
	this->SetDepthRange(0, 0);
	//d3ddev->SetFVF(D3DFVF_XYZ);
	this->current_pt = PT_LINES;
	this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	this->context->DrawIndexed(24, 0, 0);

	//d3ddev->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 24, 12, (void *)index, D3DFMT_INDEX16, ObjectBound, sizeof(D3DXVECTOR3));
	//this->SetShader(old);
	this->SetDepthRange(0, 1);
}

void CRenderer::DrawBoundingBox(const Vec3 min, const Vec3 max)//Vec3* ObjectBound)//needs vector of all 4 sides
{
	const unsigned short index[] = { 0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		2, 7,//3, 7,//nope
		1, 6,//2, 6,//nope
		3, 4,//0, 4,//nope
		0, 5 };//1, 5};//nope

	struct vertz
	{
		Vec3 p;
		unsigned int color;
	};

	vertz ObjectBound[8];
	ObjectBound[0].p = min;					   //000
	ObjectBound[1].p = Vec3(max.x, min.y, min.z);//100
	ObjectBound[2].p = Vec3(max.x, max.y, min.z);//110
	ObjectBound[3].p = Vec3(min.x, max.y, min.z);//010
	ObjectBound[4].p = Vec3(min.x, max.y, max.z);//011
	ObjectBound[5].p = Vec3(min.x, min.y, max.z);//001
	ObjectBound[6].p = Vec3(max.x, min.y, max.z);//101
	ObjectBound[7].p = max;					   //111

	for (auto& ii : ObjectBound)
		ii.color = 0xFFFFFFFF;

	//auto old = this->shader;
	this->SetShader(this->passthrough);

	CIndexBuffer ib;
	CVertexBuffer vb;
	vb.Data(ObjectBound, 16 * 8, 16);
	ib.Data((void*)index, 12 * 2 * 2, 0);

	VertexElement elm2[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR } };
	vb.SetVertexDeclaration(this->GetVertexDeclaration(elm2,2));

	this->shader->BindIL(&vb.vd);

	vb.Bind();
	ib.Bind();

	//set constant buffer
	auto mat = (Matrix4::Identity()*this->view*this->projection).Transpose();
	passthrough->buffers.wvp.UploadAndSet(&mat, sizeof(mat));

	//this->SetCullmode(CULL_NONE);
	this->SetDepthRange(0, 0);
	//d3ddev->SetFVF(D3DFVF_XYZ);
	this->current_pt = PT_LINES;
	this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	this->context->DrawIndexed(24, 0, 0);

	//d3ddev->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 24, 12, (void *)index, D3DFMT_INDEX16, ObjectBound, sizeof(D3DXVECTOR3));
	//this->SetShader(old);
	this->SetDepthRange(0, 1);
}

int i = 0;
void CRenderer::FlushDebug()
{
	//renderer->SetDepthRange(0,0);
	//Matrix4 temp = Matrix4::Identity();
	//renderer->SetMatrix(WORLD_MATRIX, &temp);
	//renderer->SetMatrix(VIEW_MATRIX, &this->view);
	debugs.lock();
	int size = debugs.size();
	for (auto ii = debugs.begin(); ii != debugs.end(); ii++)
	{
		cmd c = *ii;
#ifndef USEOPENGL
		this->DrawBoundingBox(c.b, c.color);
#endif
	}
	if (i++ == 3)
	{
		i = 0;
		debugs.clear();
	}
	debugs.unlock();

	//renderer->SetDepthRange(0,1);
}


//lets cheat, and just add all of the beams to render to a list and render last
struct Beam
{
	Vec3 start, end;
	float size;
	int color;
	CCamera* cam;
};

std::vector<Beam> beams;
void CRenderer::DrawBeams()
{
	renderer->SetPixelTexture(0, resources.get<CTexture>("Laser.png"));
	struct vertz
	{
		Vec3 p;
		unsigned int color;

		float u, v;
	};
	vertz ObjectBound[4];
	ObjectBound[0].u = 0; ObjectBound[0].v = 0;
	ObjectBound[1].u = 0; ObjectBound[1].v = 1;
	ObjectBound[2].u = 1; ObjectBound[2].v = 0;
	ObjectBound[3].u = 1; ObjectBound[3].v = 1;

	//auto old = this->shader;
	//if (this->current_texture)
	this->SetShader(this->unlit_textured);
	//else
	//	this->SetShader(this->passthrough);
	this->EnableAlphaBlending(true);
	CVertexBuffer vb;
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };
	vb.SetVertexDeclaration(this->GetVertexDeclaration(elm3, 3/*2*/));

	//this->shader->BindIL(vb.vd);

	this->SetCullmode(CULL_NONE);

	this->current_pt = PT_TRIANGLESTRIP;
	this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	auto mat = (Matrix4::Identity()*this->view*this->projection).Transpose();
	passthrough->buffers.wvp.UploadAndSet(&mat, sizeof(mat));

	for (auto beam : beams)
	{
		Vec3 major = beam.end - beam.start;
		Vec3 minor;

		Vec3 mid = (beam.end + beam.start)*0.5f;
		Vec3 dir = mid - beam.cam->_pos;// _lookAt;
		minor = major.cross(dir);
		minor.normalize();

		//Vec3 major = dir;
		Vec3 right = minor;

		//Vec3 dir = (beam.start - beam.end).getnormal();
		//Vec3 right = beam.cam->_lookAt.cross(dir).getnormal();
		if (abs(beam.cam->_upDir.dot(right)) > 0.99f)
			right = beam.cam->_right;

		//Vec3 diff = beam.cam->_pos - beam.start;
		//right = dir.cross(diff).getnormal();
		//right = dir.cross(right).getnormal();
		Vec3 off = right*(beam.size*0.5f);
		ObjectBound[0].p = beam.start - off;//min;					   //000
		ObjectBound[1].p = beam.start + off;//Vec3(max.x, min.y, min.z);//100
		ObjectBound[2].p = beam.end - off;//Vec3(max.x, max.y, min.z);//110
		ObjectBound[3].p = beam.end + off;//Vec3(min.x, max.y, min.z);//010

		for (auto& ii : ObjectBound)
			ii.color = beam.color;

		vb.Data(ObjectBound, 24 * 4, 24);
		vb.Bind();

		//set constant buffer

		//vb.SetVertexDeclaration(this->GetVertexDeclaration(3));

		this->context->Draw(4, 0);
	}
	beams.clear();
}

void CRenderer::DrawBeam(CCamera* cam, const Vec3& start, const Vec3& end, float size, unsigned int color)
{
	beams.push_back({ start, end, size, color, cam });
	return;
	Vec3 dir = (start - end).getnormal();
	Vec3 right = cam->_lookAt.cross(dir).getnormal();
	if (cam->_lookAt.dot(dir) < 0.05f)
		right = cam->_right;
	struct vertz
	{
		Vec3 p;
		unsigned int color;

		float u, v;
	};

	Vec3 min = start;
	Vec3 max = end;

	Vec3 mid = (start + end)*0.5f;
	dir = mid - cam->_pos;

	//Vec3 diff = cam->_pos - start;

	//right = dir.cross(diff).getnormal();
	right = dir.cross(right).getnormal();
	Vec3 off = right*(size*0.5f);

	vertz ObjectBound[4];
	ObjectBound[0].p = start - off;//min;					   //000
	ObjectBound[0].u = 0; ObjectBound[0].v = 0;

	ObjectBound[1].p = start + off;//Vec3(max.x, min.y, min.z);//100
	ObjectBound[1].u = 0; ObjectBound[1].v = 1;

	ObjectBound[2].p = end - off;//Vec3(max.x, max.y, min.z);//110
	ObjectBound[2].u = 1; ObjectBound[2].v = 0;

	ObjectBound[3].p = end + off;//Vec3(min.x, max.y, min.z);//010
	ObjectBound[3].u = 1; ObjectBound[3].v = 1;

	for (auto& ii : ObjectBound)
		ii.color = color;

	//auto old = this->shader;
	//if (this->current_texture)
	this->SetShader(this->unlit_textured);
	//else
	//	this->SetShader(this->passthrough);
	this->EnableAlphaBlending(true);
	CVertexBuffer vb;
	vb.Data(ObjectBound, 24 * 4, 24);
	// vb.SetVertexDeclaration(this->GetVertexDeclaration(3/*2*/));

	this->shader->BindIL(&vb.vd);

	vb.Bind();

	//set constant buffer
	auto mat = (Matrix4::Identity()*this->view*this->projection).Transpose();
	passthrough->buffers.wvp.UploadAndSet(&mat, sizeof(mat));
	VertexElement elm3[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_COLOR, USAGE_COLOR },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	vb.SetVertexDeclaration(this->GetVertexDeclaration(elm3,3));

	this->SetCullmode(CULL_NONE);

	this->current_pt = PT_TRIANGLESTRIP;
	this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	this->context->Draw(4, 0);
}

void CRenderer::SetBlendMode(BlendMode mode)
{
	float blendfactor[4] = { 1, 1, 1, 1 };
	switch (mode)
	{
	case BlendAlpha:
		context->OMSetBlendState(bs_alpha, blendfactor, 0xFFFFFFFF);
		break;
	case BlendNone:
		context->OMSetBlendState(bs_solid, blendfactor, 0xFFFFFFFF);
		break;
	case BlendAdditive:
		context->OMSetBlendState(bs_additive, blendfactor, 0xFFFFFFFF);
		break;
	case BlendSubtractive:
		context->OMSetBlendState(bs_subtractive, blendfactor, 0xFFFFFFFF);
		break;
	}
}

void CRenderer::SetPrimitiveType(enum PrimitiveType mode)
{
	if (mode != this->current_pt)
	{
		if (mode == PT_TRIANGLELIST)
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		this->current_pt = mode;
	}
}