#ifndef RENDERER_HEADER
#define RENDERER_HEADER

#include "../Defines.h"
#include "../ResourceManager.h"

class ModelData;

#include <map>
#include <string>
#include <vector>

#include "../Util/list.h"
#include "../Math/OBB.h"
#include "../Iqm.h"


struct Rect
{
	int left, top, right, bottom;
	Rect(int top, int bottom, int left, int right) : top(top), bottom(bottom), left(left), right(right)
	{

	}

	Rect() {}
	/*void operator += (int x)
	{
	this->top += y;
	this->bottom += y;
	this->left += x;
	this->right += y;
	}*/

	inline int Width()
	{
		return this->right - this->left;
	}

	inline int Height()
	{
		return this->bottom - this->top;
	}
};

#include "../defines.h"
#include "../Math/geom.h"

#include "../Math/AABB.h"

class Model;
class ResourceManager;

#define HLSL(string) #string
#define GLSL(string) #string


#ifdef ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

extern AAssetManager* mgr;
#endif

class CRenderer;
class CVertexBuffer;
class CIndexBuffer;
class CShader;
class CCamera;
class Font;

extern CRenderer* renderer;

#include "CVertexBuffer.h"

#ifndef USEOPENGL
enum FilterMode
{
	Point = 0,//D3DTEXF_POINT,
	Linear = 1,//D3DTEXF_LINEAR,
	Anisotropic = 2,//D3DTEXF_ANISOTROPIC
};
#else
enum FilterMode
{
	Point = GL_NEAREST,
	Linear = GL_LINEAR
	Anisotropic, GL_ANISOTROPIC
};
#endif

typedef unsigned long COLOR;
#ifndef _WIN32
#define COLOR_ARGB(a,r,g,b) \
	((COLOR)((((a)&0xff)<<24)|(((r)&0xff))|(((g)&0xff)<<8)|(((b)&0xff)<<16)))
#else
#define COLOR_ARGB(a,r,g,b) \
	((COLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif
#define COLOR_RGBA(r,g,b,a) COLOR_ARGB(a,r,g,b)
#define COLOR_XRGB(r,g,b)   COLOR_ARGB(0xff,r,g,b)
#define COLOR_TO_FLOAT(i) Vec4(((float)(((i)>>16)&0xff))/256.0f,((float)(((i)>>8)&0xff))/256.0f,((float)((i)&0xff))/256.0f,((float)(((i)>>24)&0xff))/256.0f)

enum PrimitiveType
{
#ifndef USEOPENGL
	PT_TRIANGLELIST = 0,//D3DPT_TRIANGLELIST,
	PT_TRIANGLESTRIP = 1,//D3DPT_TRIANGLESTRIP,
	PT_TRIANGLEFAN = 2,//D3DPT_TRIANGLEFAN,
	PT_POINTS = 3,//D3DPT_POINTLIST,
	PT_LINES = 4,//D3DPT_LINELIST,
	PT_LINESTRIP = 5,//D3DPT_LINESTRIP,

	PT_NONE = 100,
#else
	PT_TRIANGLELIST = GL_TRIANGLES,//D3DPT_TRIANGLELIST,
	PT_TRIANGLESTRIP = 2,//D3DPT_TRIANGLESTRIP,
	PT_TRIANGLEFAN = 3,//D3DPT_TRIANGLEFAN,
	PT_POINTS = 4,//D3DPT_POINTLIST,
	PT_LINES = 5,//D3DPT_LINELIST,
	PT_LINESTRIP = 6,//D3DPT_LINESTRIP,
#endif
};

enum CullMode
{
#ifndef USEOPENGL
	CULL_NONE = 0,//D3DCULL_NONE,
	CULL_CW = 1,//D3DCULL_CW,
	CULL_CCW = 2,//D3DCULL_CCW,
#else
	CULL_CW,
	CULL_CCW,
	CULL_NONE
#endif
};

enum VELEMENT_TYPE
{
	ELEMENT_FLOAT = 0,
	ELEMENT_FLOAT2 = 1,
	ELEMENT_FLOAT3 = 2,
	ELEMENT_FLOAT4 = 3,
	ELEMENT_UBYTE4 = 4,
	ELEMENT_COLOR = 5
};

enum VELEMENT_USAGE
{
	USAGE_POSITION,
	USAGE_TEXCOORD,
	USAGE_NORMAL,
	USAGE_TANGENT,
	USAGE_COLOR,
	USAGE_BLENDINDICES,
	USAGE_BLENDWEIGHT,
	USAGE_NONE//used to specify something not used in the shader
};

struct VertexElement
{
	VELEMENT_TYPE type;
	VELEMENT_USAGE usage;
};

enum Blend
{
	Zero = 0,//D3DBLEND_ZERO,
	One = 1,//D3DBLEND_ONE,
	SrcColor = 2,//D3DBLEND_SRCCOLOR,
	InvSrcColor = 3,//D3DBLEND_INVSRCCOLOR,
	SrcAlpha,// = D3DBLEND_SRCALPHA,
	InvSrcAlpha,// = D3DBLEND_INVSRCALPHA,
	DestAlpha,// = D3DBLEND_DESTALPHA,
	InvDestAlpha,// = D3DBLEND_INVDESTALPHA,
	DestColor,// = D3DBLEND_DESTCOLOR,
	InvDestColor,// = D3DBLEND_INVDESTCOLOR,
};

#ifdef USEOPENGL
struct _VertexElement
{
	int Method;
	int Type;
	int Usage;
	int Stream;
	int Offset;
	int UsageIndex;
};
#endif

#define WORLD_MATRIX 1
#define VIEW_MATRIX 2
#define PROJECTION_MATRIX 3
#define WVP_MATRIX 4

//font alignment
#define ALIGN_RIGHT 8

class CTexture;
class Parent;
class CRenderTexture;

struct D3D11_INPUT_ELEMENT_DESC;

interface ID3D11SamplerState;
interface ID3D11BlendState;
interface ID3D11RasterizerState;
interface ID3D11Device;
interface ID3D11DeviceContext;
interface IDXGISwapChain;
interface ID3D11ShaderResourceView;
interface ID3D11DepthStencilView;
interface ID3D11RenderTargetView;
interface ID3D11DepthStencilState;

//typedefs
#ifndef USEOPENGL
typedef ID3D11ShaderResourceView* Texture;
#else
typedef int D3DCOLOR;
typedef unsigned int Texture;
#endif
#undef DrawText
struct IDXGIFactory;
class Window;
class CRenderer
{
	friend class Renderable;
	friend class Renderer;
	friend class ChunkRenderable;
	friend class CVertexBuffer;
	friend class CIndexBuffer;
	friend class CTexture;
	friend class CCamera;
	friend class CWorld;
	friend class Font;
	friend class CShader;

	ID3D11BlendState* bs_solid;
	ID3D11BlendState* bs_alpha;
	ID3D11BlendState* bs_additive;
	ID3D11BlendState* bs_subtractive;
	ID3D11BlendState* bs_font;

	ID3D11RasterizerState* rs_cw;
	ID3D11RasterizerState* rs_ccw;
	ID3D11RasterizerState* rs_none;
	ID3D11RasterizerState* rs_wireframe;
public:
	ID3D11RasterizerState* rs_scissor;
private:

	ID3D11SamplerState* point_sampler;
	ID3D11SamplerState* linear_sampler;

	ID3D11DepthStencilState* depthStencilStateNoWrite, *depthStencilState;

	ID3D11ShaderResourceView* missing_texture = 0;

	bool wireframe;
	bool vsync;

	PrimitiveType current_pt = PrimitiveType::PT_NONE;

	struct VertexElementCache
	{
		VertexElement* data;
		int size;
		int key;
		VertexDeclaration vd;
	};
	std::vector<VertexElementCache> vaos;

	HWND wnd;

	IDXGISwapChain* chain = 0;

	CVertexBuffer rectangle_v_buffer;

	VertexDeclaration input_layout;
public:

	ID3D11Device* device;
	ID3D11DeviceContext* context;

	int current_aa_level = -1;
	IDXGIFactory* factory;

	CShader* shader;//the current shader being used

	Matrix4 world, view, projection;

private:

	CShader* passthrough = 0;
	CShader* unlit_textured = 0;

	CShader* shaders[25];

public:
	//change shaders to not use magic numbers anymore and name them
	CTexture* gui_texture;

	int xres;
	int yres;

	CRenderer();
	~CRenderer();


	//todo remove this, this shouldnt be here
	int pos;
	struct Value
	{
		bool text;
		float value;
	};
	Value values[200];
	void AddPoint(float value);//adds point to the debug graph
	void DrawGraph();//renders the debug graph

	void SetAALevel(int samples);

#ifdef _WIN32
	void Init(HWND hWnd, int scrx, int scry);
	void Init(Window* wnd, int scrx, int scry);
#else
	void Init(int scrx, int scry);
#endif
	void EnableVsync(bool tf)
	{
		if (tf != vsync)
		{
			vsync = tf;

			this->Resize(renderer->xres, renderer->yres);
		}
	}
	bool Vsync() { return this->vsync; }

	void Resize(int scrx, int scry);
	void Present();

	void DrawFullScreenQuad();
	void DrawRect(Rect* rct, COLOR color, bool setshader = true);
	void DrawRectUV(Rect* rct, float minu, float maxu, float minv, float maxv, COLOR vertexColor, bool set_shader = true);
	void DrawRectUV(Rect* rct, Vec2 top_left, Vec2 top_right, Vec2 bottom_left, Vec2 bottom_right, COLOR color, bool set_shader = true);

	void ClipToScreenPosition(float &x, float &y);
	void ScreenToClipPosition(float &x, float &y);

	void GetViewport(Viewport* vp);
	void SetViewport(Viewport* vp);

	//text rendering related stuff
private:
	Font* font; int fontsize;
public:
	void SetFont(char* name, int size);

	Rect DrawText(Rect r, const char* text, COLOR color, int flags = 0);//does auto line breaks
	void DrawText(int x, int y, const char* text, COLOR color);
	void DrawVerticalCenteredText(Rect r, const char* text, COLOR color);
	void DrawCenteredText(Rect r, const char* text, COLOR color);

	int TextSize(const char* txt, int n = 0);


	void SetPrimitiveType(enum PrimitiveType pt);

	void EnableWireframe(bool b)
	{
		this->wireframe = b;
	}

	//debug stuff
private:
	struct cmd
	{
		int type;
		OBB b;
		COLOR color;
		//for lines
		Vec3 start, end;
	};
	List<cmd> debugs;
public:
	void DebugDrawOBB(OBB b, COLOR color = COLOR_ARGB(255, 255, 255, 255))
	{
		cmd c;
		c.type = 0;
		c.b = b;
		c.color = color;
		debugs.push_back(c);
	}
	void DebugDrawLine(Vec3 start, Vec3 end, COLOR color)
	{
		cmd c;
		c.type = 1;
		c.start = start;
		c.end = end;
		c.color = color;
		debugs.push_back(c);
	}
	void FlushDebug();
	Vec3 cam_pos;

	//shader related stuff
	CShader* SetShader(CShader* shader);

private:
	//shaders with numeric ids are only for internal CRenderer use as they are being phased out
	CShader* SetShader(int id);
	CShader* CreateShader(int id, const char* filename);
#ifdef _WIN32
	void CreateShader(int id, char* vloc, char* vfunc, char* ploc, char* pfunc);
#endif

public:
	VertexDeclaration GetVertexDeclaration(VertexElement* elm, unsigned int count);

	void SetVertexDeclaration(VertexDeclaration vd)
	{
		this->input_layout = vd;
	}

	ID3D11RenderTargetView* renderTargetView = 0;
	ID3D11DepthStencilView* depthStencilView = 0;

	void StencilFunc(unsigned int func, unsigned int ref, unsigned int mask);
	void StencilOp(int fail, int zfail, int zpass);
	void StencilMask(unsigned int mask);

	void DepthWriteEnable(bool on);
	void SetDepthRange(float Near, float Far);

	CVertexBuffer* MakeCubeVB(float scale, int blockID, COLOR color);

	void Clear(bool cleardepth, bool clearstencil);
	void Clear(float a, float r, float g, float b);

	void ClearDepth();
	void ClearStencil();

	ID3D11ShaderResourceView* current_texture = 0;
	void SetPixelTexture(int stage, int id)
	{
		this->current_texture = 0;
	}
	void SetPixelTexture(int stage, ID3D11ShaderResourceView* tex);
	void SetVertexTexture(int stage, ID3D11ShaderResourceView* tex);
	void SetPixelTexture(int stage, CTexture* tex);
	void SetVertexTexture(int stage, CTexture* tex);

	CRenderTexture GetRenderTarget(int id);
	void SetRenderTarget(int id, const CRenderTexture* rt);
	void SetFilter(int stage, FilterMode mode);

	void EnableAlphaBlending(bool yes);

	enum BlendMode
	{
		BlendAdditive,
		BlendSubtractive,
		BlendAlpha,
		BlendNone,
	};
	void SetBlendMode(BlendMode mode);

	//blending config
	void SetSrcBlend(Blend mode);
	void SetDestBlend(Blend mode);

	bool Override;
	void SetCullmode(enum CullMode mode);
	void SetCullmode(enum CullMode mode, bool oride);

	void SetMatrix(int type, const Matrix4* mat);

	//use for drawing with shaders
	void DrawPrimitive(enum PrimitiveType mode, unsigned int offset, unsigned int vertices);
	void DrawIndexedPrimitive(enum PrimitiveType mode, unsigned int minvertexindex, unsigned int startindex, unsigned int vertices, unsigned int numindices);//(enum PrimitiveType mode, unsigned int offset, unsigned int vertices, unsigned int primitives);


	void DrawIcon(int x, int y, int size, int id, COLOR color = COLOR_ARGB(255, 255, 255, 255));

	void ApplyCam(CCamera* cam);

	void DrawBoundingBox(const OBB bb, COLOR color = COLOR_ARGB(255, 255, 255, 255));

	void DrawBoundingBox(const AABB bb)
	{
		this->DrawBoundingBox(bb.min, bb.max);
	}

	void DrawNormals(Vec3 pos, Vec3 x, Vec3 y, Vec3 z);

	void DrawFrustum(const Matrix4& view, const Matrix4& proj, COLOR color);

	void DrawBoundingBox(const Vec3 min, const Vec3 max);

	//returns false if the position is onscreen
	bool WorldToScreen(CCamera* cam, const Vec3 pos, Vec3& out, Parent* p = 0);

	//stats stuff
	struct RendererStats
	{
		unsigned int shader_changes;
		unsigned int vertices;
		unsigned int triangles;
		unsigned int drawcalls;
		unsigned int vertexbuffers;
		unsigned int vertexbuffer_mem;
		unsigned int textures;//todo
		unsigned int texture_mem;//todo
	};
	RendererStats stats;

	void ResetStats()
	{
		stats.shader_changes = stats.vertices = stats.drawcalls = stats.triangles = 0;
	}

	void DrawBeams();
	void DrawBeam(CCamera* cam, const Vec3& start, const Vec3& end, float size, unsigned int color);

	//todo move this somewhere else
	std::string FormatMemory(unsigned int size)
	{
		char temp[50];
		double vbmem = size;
		const char* ending = "B";
		if (vbmem > 10 * 1024)
		{
			vbmem /= 1024;
			ending = "KB";
		}
		if (vbmem > 10 * 1024)
		{
			vbmem /= 1024;
			ending = "MB";
		}
		if (vbmem > 10 * 1024)
		{
			vbmem /= 1024;
			ending = "GB";
		}

		if (vbmem > 1000)
		{
			sprintf(temp, "%.0f,%0.1f%s", vbmem / 1000.0, fmod(vbmem, 1000.0), ending);
		}
		else
		{
			sprintf(temp, "%0.1f%s", vbmem, ending);
		}
		return std::string(temp);
	}

	void DrawStats(float frametime, float realframetime, unsigned int memuse, float rp);

	ID3D11ShaderResourceView* GetMissingTextureImage();
};
#endif