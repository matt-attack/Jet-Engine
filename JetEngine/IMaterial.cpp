#include "IMaterial.h"
#include "Graphics/CTexture.h"
#include "Graphics/Shader.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderable.h"
#include "ResourceManager.h"

std::map<std::string, IMaterial*> materials;

const int num_builder_options = 5;
class ShaderBuilder : public Resource
{
	std::string path;

public:
	CShader* shaders[1 << num_builder_options];

	ShaderBuilder()
	{
		for (int i = 0; i < (1 << num_builder_options); i++)
			shaders[i] = 0;
	}

	~ShaderBuilder()
	{
		for (int i = 0; i < (1 << num_builder_options); i++)
			delete shaders[i];
	}

	virtual void Reload(ResourceManager* mgr, const std::string& path)
	{
		if (this->path.length())//shaders[0])
		{
			for (int i = 0; i < (1 << num_builder_options); i++)
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

		char* list[num_builder_options];
		char* options[] = { "SKINNING", "NORMAL_MAP", "POINT_LIGHTS", "SHADOWS", "ALPHA_TEST" };
		//build the list
		int size = 0;
		for (int i = 0; i < num_builder_options; i++)
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
	if (renderer && renderer->passthrough)//rectangle_v_buffer.GetSize() > 0)//HAX lel
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

	//todo: dont set shader here, need to be able to get the right shader
	//for the job later based on lighting
	//renderer->SetShader(shader_ptr);

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

		int id = (this->alphatest ? ALPHA_TEST : 0) |
			(this->normal_map ? NORMAL_MAP : 0) |
			(this->skinned ? SKINNING : 0) |
			0;// (POINT_LIGHTS);

		if (r._shadows)
			id |= SHADOWS;

		this->shader_ptr = shaders->GetShader(id);
		this->shader_lit_ptr = shaders->GetShader(id | POINT_LIGHTS);
		this->shader_unskinned_ptr = shaders->GetShader(id ^ SKINNING);
		this->shader_lit_unskinned_ptr = shaders->GetShader((id | POINT_LIGHTS) ^ SKINNING);

	}
	else
	{
		this->shader_ptr = resources.get<CShader>(this->shader_name);
	}
}

void IMaterial::ApplyShader(bool skinned, bool lit)
{
	if (this->shader_builder == false)
	{
		renderer->SetShader(shader_ptr);
		return;
	}

	if (skinned)
		if (lit)
			renderer->SetShader(shader_lit_ptr);
		else
			renderer->SetShader(shader_ptr);
	else
		if (lit)
			renderer->SetShader(shader_lit_unskinned_ptr);
		else
			renderer->SetShader(shader_unskinned_ptr);
}


//loads a material from a .mat file
#include <fstream>
#include <sstream>

bool read_bool(const std::string& in)
{
	if (in == "false")
		return false;
	return true;
}
IMaterial* IMaterial::Load(const char* name)
{
	ok, lets get auto material reloading
	auto mat = new IMaterial((char*)name);

	//setup defaults here
	mat->alpha = false;
	mat->alphatest = false;
	mat->filter = FilterMode::Linear;
	mat->depthhack = false;
	mat->cullmode = CULL_CW;

	mat->shader_name = "Shaders/ubershader.txt";// "Shaders/generic.txt";
	mat->shader_builder = true;
	mat->skinned = true;//this doesnt always need to be true

	std::string realname = "Content/";
	realname += name;
	realname += ".mat";
	auto file = std::ifstream(realname);
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string a, b;
		if (!(iss >> a >> b)) { break; } // error
	
		// process pair (a,b)
		if (a == "normal:")
			mat->normal = b;
		else if (a == "diffuse:")
			mat->diffuse = b;
		else if (a == "alpha:")
			mat->alphatest = read_bool(b);//fixme
		//else if (a == "cast_shadows:")
		//	mat->
	}
	return mat;
}