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

private:
	std::map<std::string, std::string> defines;//set whenever you call LoadShader with those defines
public:

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
					this->LoadShader(i, this->defines);

			return;
		}
		this->path = path;
	}

	void InvalidateCache()
	{
		for (int i = 0; i < (1 << num_builder_options); i++)
			if (this->shaders[i])
			{
				delete this->shaders[i];
				this->shaders[i] = 0;
 			}
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

	CShader* GetShader(int id, std::map<std::string, std::string>& defines)
	{
		if (this->shaders[id])
			return this->shaders[id];

		this->LoadShader(id, defines);

		return this->shaders[id];
	}

	void LoadShader(int id, std::map<std::string, std::string>& defines)
	{
		if (this->shaders[id] == 0)
			this->shaders[id] = new CShader;

		this->defines = defines;

		//add defines to definitions list
		char** list = (char**)_alloca(sizeof(char*)*(num_builder_options + 1 + defines.size()));// [num_builder_options + 1];
		char** definitions = (char**)_alloca(sizeof(char*)*(num_builder_options + 1 + defines.size())); //[num_builder_options + 1];
		char* options[] = { "SKINNING", "NORMAL_MAP", "POINT_LIGHTS", "SHADOWS", "ALPHA_TEST" };
		//build the list
		int size = 0;
		for (int i = 0; i < num_builder_options; i++)
			if (id & (1 << i))
			{
				list[size] = options[i];
				definitions[size++] = "true";
			}

		if (id & (1 << 2))//if point lights, specify a number we want
		{
			list[size] = "NUM_POINTS";
			definitions[size++] = "3";
		}

		//add extra definitions
		for (auto ii : this->defines)
		{
			list[size] = new char[ii.first.size() + 1];// (char*)ii.first.c_str();
			strcpy(list[size], ii.first.c_str());
			definitions[size] = new char[ii.second.size() + 1];// (char*)ii.second.c_str();
			strcpy(definitions[size++], ii.second.c_str());
		}

		auto shader = CShader(path.c_str(), "vs_main", path.c_str(), "ps_main", list, definitions, size);

		//free the defines
		for (int i = size; i > size - this->defines.size(); i--)
		{
			delete[] list[i-1];
			delete[] definitions[i-1];
		}

		if (shader.vshader == 0 || shader.pshader == 0)
			return;//epic fail

		*this->shaders[id] = shader;
	}
};

IMaterial::IMaterial(const char* name)
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

IMaterial::~IMaterial()
{
	if (this->texture)
		this->texture->Release();

	if (this->normal_map)
		this->normal_map->Release();
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
		auto tex = resources.get_unsafe<CTexture>(diffuse);
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
		auto tex = resources.get_unsafe<CTexture>(normal);
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
		auto shaders = resources.get_unsafe<ShaderBuilder>(this->shader_name);

		//these are static material values
		int id = (this->alphatest ? ALPHA_TEST : 0) |
			(this->normal_map ? NORMAL_MAP : 0) |
			(this->skinned ? SKINNING : 0) |
			0;// (POINT_LIGHTS);

		if (r._shadows)
			id |= SHADOWS;

		//these are shaders that will be used depending on lighting and scene configurations
		this->shader_ptr = shaders->GetShader(id, this->defines);
		this->shader_lit_ptr = shaders->GetShader(id | POINT_LIGHTS, this->defines);
		this->shader_unskinned_ptr = shaders->GetShader(id & (~SKINNING), this->defines);
		this->shader_lit_unskinned_ptr = shaders->GetShader((id | POINT_LIGHTS) & (~SKINNING), this->defines);

		shaders->Release();
	}
	else
	{
		//todo: get passing defines to work here
		auto shdr = resources.get_unsafe<CShader>(this->shader_name);
		this->shader_ptr = shdr;
	}
}

void IMaterial::ApplyShader(bool skinned, int lights)
{
	if (this->shader_builder == false)
	{
		renderer->SetShader(shader_ptr);
		return;
	}

	//todo: ok, have different shaders for different numbers of lights

	if (skinned)
		if (lights)
			renderer->SetShader(shader_lit_ptr);
		else
			renderer->SetShader(shader_ptr);
	else
		if (lights)
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
IMaterial* IMaterial::load_as_resource(const std::string &path, IMaterial* res)//Load(const char* name)
{
	//ok, lets get auto material reloading
	//	add alpha test to shadows
	//	and fix branch culling / lighting issues
	auto mat = res;// new IMaterial((char*)name);

	std::string name = path;
	int offset = name.find_last_of('/');
	name = name.substr(offset, name.length()-offset);

	new (mat) IMaterial(name.c_str());
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
			mat->alpha = read_bool(b);
		else if (a == "alpha_test:")
			mat->alphatest = read_bool(b);
		else if (a == "cull:")
			if (!read_bool(b))
				mat->cullmode = CULL_NONE;
		//else if (a == "cast_shadows:")
		//	mat->
	}
	return mat;
}


void IMaterial::SetDefine(const std::string& name, const std::string& value)
{
	this->defines[name] = value;

	auto shaders = resources.get_unsafe<ShaderBuilder>(this->shader_name);

	//apply the changes
	shaders->InvalidateCache();	
}