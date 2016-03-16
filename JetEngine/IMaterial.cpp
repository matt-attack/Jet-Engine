#include "IMaterial.h"
#include "Graphics\CTexture.h"
#include "Graphics\Shader.h"
#include "Graphics\Renderer.h"

std::map<std::string, IMaterial*> materials;

class ShaderBuilder : public Resource
{
public:
	CShader* skinned = 0;
	CShader* normal = 0;

	ShaderBuilder()
	{
	}
	~ShaderBuilder()
	{
	}

	virtual void Reload(ResourceManager* mgr, const std::string& path)
	{
		char* p[] = { "SKINNING" };
		if (this->skinned)
		{
			*this->skinned = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p, 1);
			*this->normal = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main");
			return;
		}
		this->skinned = new CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", p, 1);
		this->normal = new CShader(path.c_str(), "vs_main", path.c_str(), "ps_main");
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

/*IMaterial::IMaterial(char* name, int shader, FilterMode fmode, char* diffuse, char* normal, CullMode cmode, bool alpha, bool weaponhack)
{
	//this->shader = shader;
	this->filter = fmode;
	this->cullmode = cmode;
	this->alpha = alpha;
	this->alphatest = false;
	this->depthhack = weaponhack;

	//add normal mapping
	//name and textures
	this->name = name;
	this->diffuse = diffuse ? diffuse : "";
	this->normal = normal ? normal : "";

	//need to update here if possible
	if (renderer->rectangle_v_buffer.GetSize() > 0)//HAX lel
		this->Update(renderer);

	GetList()[name] = this;
}*/

void IMaterial::Apply(CRenderer* renderer)
{
	//make me smarter at some point and not just blindly set everything, implement this in the renderer class
	renderer->SetCullmode(cullmode);

	renderer->SetPixelTexture(0, this->texture);

	renderer->EnableAlphaBlending(alpha);
	renderer->SetFilter(0, this->filter);

	//if (shader)
		//renderer->SetShader(shader);//o shoot, we were already doing this here?

	//if (this->shader_ptr)
		renderer->SetShader(shader_ptr);

	//figure out shader setup, where should what go?
	//need skinned/unskinned versions


	//also textures need to be moved to materials
	if (this->depthhack)
		renderer->SetDepthRange(0,0.1f);
	else
		renderer->SetDepthRange(0,1);
}

#include "Graphics\Renderable.h"
#include "ResourceManager.h"
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

	if (this->shader_builder)
	{
		auto shaders = resources.get<ShaderBuilder>(this->shader_name);
		auto shaders_s = resources.get<ShaderBuilder>("Shaders/shadowed.txt");

		if (r._shadows)
			shaders = shaders_s;

		if (this->skinned)
			this->shader_ptr = shaders->skinned;
		else
			this->shader_ptr = shaders->normal;
	}
	else
	{
		this->shader_ptr = resources.get<CShader>(this->shader_name);
	}
}