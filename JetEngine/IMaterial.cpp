#include "IMaterial.h"
#include "Graphics/CTexture.h"
#include "Graphics/Shader.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderable.h"
#include "ResourceManager.h"

std::map<std::string, IMaterial*> materials;

#define AddShader3(a,b,c) char* p##a##b##c[] = { #a, #b, #c }; *this->shaders[a|b|c] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p##a##b##c, 3)
#define AddShader2(a,b) char* p##a##b[] = { #a, #b }; *this->shaders[a|b] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p##a##b, 2)
#define AddShader1(a) char* p##a[] = { #a }; *this->shaders[a] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p##a, 1)
#define AddShader0() *this->shaders[0] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", 0, 0)

#define Add1Shader3(a,b,c) case a|b|c:  {char* p##a##b##c[] = { #a, #b, #c }; *this->shaders[a|b|c] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p##a##b##c, 3); break;}
#define Add1Shader2(a,b) case a|b: {char* p##a##b[] = { #a, #b }; *this->shaders[a|b] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p##a##b, 2); break;}
#define Add1Shader1(a) case a: {char* p##a[] = { #a }; *this->shaders[a] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p##a, 1); break;}
#define Add1Shader0() case 0: {*this->shaders[0] = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", 0, 0); break;}

class ShaderBuilder : public Resource
{
	std::string path;

public:
	CShader* shaders[8];

	ShaderBuilder()
	{
		for (int i = 0; i < 8; i++)
			shaders[i] = 0;
	}

	~ShaderBuilder()
	{
		for (int i = 0; i < 8; i++)
			delete shaders[i];
	}

	virtual void Reload(ResourceManager* mgr, const std::string& path)
	{
		if (this->path.length())//shaders[0])
		{
			for (int i = 0; i < 8; i++)
				if (this->shaders[i])
					this->LoadShader(i);

			return;
		}
		this->path = path;
	}

	static ShaderBuilder* load_as_resource(const std::string &path, ShaderBuilder* res)
	{
		ShaderBuilder* d = res;

		//construct shaders
		d->Reload(d->resmgr(), path);

		//load with Jet
		//have a table with opengl, dx, and various settings

		return d;
	}

	CShader* GetShader(int id)
	{
		if (this->shaders[id])
			return this->shaders[id];

		this->LoadShader(id);

		return this->shaders[id];
	}

	void LoadShader(int id)
	{
		if (this->shaders[id] == 0)
			this->shaders[id] = new CShader;

		char* list[3];
		char* options[] = { "SKINNING", "NORMAL_MAP", "POINT_LIGHTS" };
		//build the list
		int size = 0;
		for (int i = 0; i < 3; i++)
			if (id & (1 << i))
				list[size++] = options[i];
		
		auto shader = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", list, size);

		if (shader.vshader == 0 || shader.pshader == 0)
			return;//epic fail

		*this->shaders[id] = shader;
	}
};

IMaterial::IMaterial(char* name)
{
	this->cullmode = CULL_CW;
	this->texture = 0;
	this->alpha = this->alphatest = false;
	this->filter = Linear;
	this->depthhack = false;
	this->shader_ptr = 0;
	this->skinned = false;

	this->name = name;

	GetList()[name] = this;
}

IMaterial::IMaterial(char* name, char* shader, FilterMode fmode, char* diffuse, CullMode cmode, bool alpha, bool weaponhack)
{
	this->shader_name = shader;
	this->filter = fmode;
	this->cullmode = cmode;
	this->alpha = alpha;
	this->alphatest = false;
	this->depthhack = weaponhack;
	this->skinned = true;

	this->shader_ptr = 0;

	//name and textures
	this->name = name;
	this->diffuse = diffuse ? diffuse : "";

	//need to update here if possible
	if (renderer && renderer->rectangle_v_buffer.GetSize() > 0)//HAX lel
		this->Update(renderer);

	GetList()[name] = this;
}

void IMaterial::Apply(CRenderer* renderer)
{
	//make me smarter at some point and not just blindly set everything, implement this in the renderer class
	renderer->SetCullmode(cullmode);

	renderer->SetPixelTexture(0, this->texture);

	if (this->normal_map)
		renderer->SetPixelTexture(10, this->normal_map);

	renderer->EnableAlphaBlending(alpha);
	renderer->SetFilter(0, this->filter);

	//if (this->shader_ptr)
	renderer->SetShader(shader_ptr);

	//also textures need to be moved to materials
	if (this->depthhack)
		renderer->SetDepthRange(0, 0.1f);
	else
		renderer->SetDepthRange(0, 1);
}

void IMaterial::Update(CRenderer* renderer)
{
	//updates internal data like shaders/textures
	if (this->diffuse.length() > 0)
	{
		//try and load
		auto tex = resources.get<CTexture>(diffuse);
		if (tex && tex->texture)
			this->texture = tex->texture;
	}
	else
	{
		//just set to null
		this->texture = 0;
	}

	if (this->normal.length() > 0)
	{
		auto tex = resources.get<CTexture>(normal);
		if (tex && tex->texture)
			this->normal_map = tex->texture;

		if (this->shader_builder)
			this->needs_tangent = true;
	}
	else
	{
		this->normal_map = 0;
	}
	if (this->shader_builder)
	{
		auto shaders = resources.get<ShaderBuilder>(this->shader_name);
		auto shaders_s = resources.get<ShaderBuilder>("Shaders/shadowed.txt");

		if (r._shadows)
			shaders = shaders_s;

		int id = (this->normal_map ? NORMAL_MAP : 0) |
			(this->skinned ? SKINNING : 0) |
			0;// (POINT_LIGHTS);

		this->shader_ptr = shaders->GetShader(id);
		this->shader_lit_ptr = shaders->GetShader(id | POINT_LIGHTS);
	}
	else
	{
		this->shader_ptr = resources.get<CShader>(this->shader_name);
	}
}