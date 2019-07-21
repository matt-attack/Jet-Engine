#include "IMaterial.h"
#include "Graphics/CTexture.h"
#include "Graphics/Shader.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderable.h"
#include "ResourceManager.h"

#include <fstream>
#include <sstream>
#include <iostream>


std::map<std::string, IMaterial*> materials;

const int num_builder_options = 6;
class ShaderBuilder : public Resource
{
	std::string path;
	std::vector<std::string> includes;
public:

	std::map<IMaterial*, CShader*[1 << num_builder_options]> shaders;

private:
	std::map<std::string, std::string> defines;//set whenever you call LoadShader with those defines
public:

	ShaderBuilder()
	{

	}

	~ShaderBuilder()
	{
		for (auto ii : shaders)
			for (int i = 0; i < 64; i++)
				delete shaders[ii.first][i];
	}

	virtual void Reload(ResourceManager* mgr, const std::string& path)
	{


		if (this->path.length())
		{
			for (auto& shader : this->shaders)
			{
				for (int i = 0; i < (1 << num_builder_options); i++)
				{
					if (shader.second[i])
					{
						delete shader.second[i];
						shader.second[i] = 0;
					}
				}
			}

			return;
		}
		this->path = path;

		std::ifstream t;
		int length;
		t.open(path, std::ios::binary | std::ios::ate);      // open input file
		t.seekg(0, std::ios::end);    // go to the end
		length = t.tellg();           // report location (this is the length)
		t.seekg(0, std::ios::beg);    // go back to the beginning
		char* buffer = new char[length + 1];    // allocate memory for a buffer of appropriate dimension
		t.read(buffer, length);       // read the whole file into the buffer
		t.close();                    // close file handle
		buffer[length] = 0;

		//look for any includes
		std::string str(buffer);
		int index = str.find("#include");
		if (index > 0)
		{
			int end = str.find('\n', index);
			std::string ipath = str.substr(index, end - index);
			//add it as a hook for reloading

			int sbegin = ipath.find('"') + 1;
			ipath = ipath.substr(sbegin, ipath.find('"', sbegin) - sbegin);
			ipath = "Shaders/" + ipath;
			resources.children[ipath] = this;
		}
	}

	void InvalidateCache()
	{
		for (auto& shader : this->shaders)
		{
			for (int i = 0; i < (1 << num_builder_options); i++)
			{
				if (shader.second[i])
				{
					delete shader.second[i];
					shader.second[i] = 0;
				}
			}
		}
	}

	void InvalidateCache(IMaterial* mat)
	{
		if (shaders.find(mat) == shaders.end())
			return;

		for (int i = 0; i < (1 << num_builder_options); i++)
		{
			if (shaders[mat][i])
			{
				delete shaders[mat][i];
				shaders[mat][i] = 0;
			}
		}
	}

	static ShaderBuilder* load_as_resource(const std::string &path, ShaderBuilder* res)
	{
		ShaderBuilder* d = res;

		//construct shaders
		d->Reload(d->resmgr(), path);

		return d;
	}
	
	//gets a shader for a material witha specific configuration number and set of defines
	CShader* GetShader(int id, const IMaterial* material)
	{
		IMaterial* key = (IMaterial*)material;
		if (material->defines.size() == 0 && material->surface_shader.length() == 0)
			key = 0;//optimization here so we dont need a custom shader made if we dont use defines or surface shader

		//todo: can optimize such that if surface shader and defines are the same that you can use the same shaders

		//get a key
		auto res = shaders.find(key);
		if (res != shaders.end() && res->second[id])
			return res->second[id];

		this->shaders[key][id] = this->LoadShader(id, material->defines, material->surface_shader);

		return this->shaders[key][id];
	}

private:
	std::string surface_shader;
	CShader* LoadShader(int id, const std::map<std::string, std::string>& defines, const std::string& surface_shader)
	{
		this->surface_shader = surface_shader;
		this->defines = defines;

		//add defines to definitions list
		char** list = (char**)_alloca(sizeof(char*)*(num_builder_options + 1 + defines.size()));
		char** definitions = (char**)_alloca(sizeof(char*)*(num_builder_options + 1 + defines.size()));
		char* options[] = { "SKINNING", "DIFFUSE_MAP", "NORMAL_MAP", "POINT_LIGHTS", "SHADOWS", "ALPHA_TEST" };
		//build the list
		int size = 0;
		for (int i = 0; i < num_builder_options; i++)
		{
			if (id & (1 << i))
			{
				list[size] = options[i];
				definitions[size++] = "true";
			}
		}

		if (id & (1 << 2))//if point lights, specify a number we want
		{
			list[size] = "NUM_POINTS";
			definitions[size++] = "3";
		}

		//add extra definitions
		for (auto ii : this->defines)
		{
			list[size] = new char[ii.first.size() + 1];
			strcpy(list[size], ii.first.c_str());
			definitions[size] = new char[ii.second.size() + 1];
			strcpy(definitions[size++], ii.second.c_str());
		}

		//load in the shader from the file
		bool add_dummy = this->surface_shader.find("SurfaceShader") == -1;
		const char empty_surface[] = "void SurfaceShader(inout VS_OUTPUT In) {}";
		std::ifstream t;
		int length;
		t.open(path, std::ios::binary | std::ios::ate);      // open input file
		t.seekg(0, std::ios::end);    // go to the end
		length = t.tellg();           // report location (this is the length)
		t.seekg(0, std::ios::beg);    // go back to the beginning
		int surface_len = !add_dummy ? this->surface_shader.length() : this->surface_shader.length() + sizeof(empty_surface);
		char* buffer = new char[length + 1 + surface_len];    // allocate memory for a buffer of appropriate dimension
		t.read(buffer, length);       // read the whole file into the buffer
		t.close();                    // close file handle
		buffer[length] = 0;

		//need to append the surface shader if we have one and use it
		if (this->surface_shader.length())
			strcpy(&buffer[length], this->surface_shader.c_str());
		
		//add dummy surface shader if one isnt set
		if (add_dummy)
		{
			strcpy(&buffer[length], empty_surface);
		}
		
		//need to fix the includes
		auto shader = new CShader(buffer, path, "vs_main", "ps_main", list, definitions, size);

		delete[] buffer;

		//free the defines
		for (int i = size; i > size - this->defines.size(); i--)
		{
			delete[] list[i - 1];
			delete[] definitions[i - 1];
		}

		if (shader->vshader == 0 || shader->pshader == 0)
			return 0;//epic fail

		return shader;
	}
};

IMaterial::IMaterial(const char* name)
{
	//set default material options
	this->SetDefaults();
	this->texture = 0;
	this->shader_ptr = 0;
	this->name = name;

	if (GetList().find(name) != GetList().end())
	{
		printf("A material with name %s already exists!", name);
		//throw 7;
	}
	GetList()[name] = this;//add myself to the material list
}

IMaterial::IMaterial(const char* name, const IMaterial* tc) : IMaterial(name)
{
	this->filter = tc->filter;
	this->diffuse = tc->diffuse;
	this->normal = tc->normal;
	this->cullmode = tc->cullmode;
	this->alpha = tc->alpha;
	this->alphatest = tc->alphatest;
	this->base_material = tc->base_material;
	this->depthhack = tc->depthhack;
	this->shader_builder = tc->shader_builder;
	this->shader_name = tc->shader_name;
	this->surface_shader = tc->surface_shader;
}

IMaterial::~IMaterial()
{
	//if (this->texture)
	//	this->texture->Release();

	//if (this->normal_map)
	//	this->normal_map->Release();
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
	if (renderer && renderer->gui_texture)//HAX lel
		this->Update(renderer);

	if (GetList().find(name) != GetList().end())
	{
		printf("A material with name %s already exists!", name);
		throw 7;
	}
	GetList()[name] = this;
}

void IMaterial::SetDefaults()
{
	this->cullmode = CULL_CW;
	this->alpha = this->alphatest = false;
	this->filter = Linear;
	this->depthhack = false;
	this->skinned = false;
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
	//todo: support generated textures that dont exist as a file
	//updates internal data like shaders/textures
	if (this->diffuse.length() > 0 && this->diffuse != "DYNAMIC")//if dynamically set texture, do nothing
	{
		//try and load
		auto tex = resources.get_unsafe<CTexture>(diffuse);
		if (tex && tex->texture_rv)
			this->texture = tex->texture_rv;
	}
	else
	{
		//just set to null
		this->texture = 0;
	}

	//load/update normal map if we have one
	if (this->normal.length() > 0)
	{
		auto tex = resources.get_unsafe<CTexture>(normal);
		if (tex && tex->texture_rv)
			this->normal_map = tex->texture_rv;

		if (this->shader_builder)
			this->needs_tangent = true;
	}
	else
	{
		this->normal_map = 0;
	}

	//build or load shaders as needed to render the material
	if (this->shader_builder)
	{
		auto shaders = resources.get_unsafe<ShaderBuilder>(this->shader_name);

		//need shaderbuilder to hold a bunch of shaders, one for each surface shader
		//these are static material values
		int id = (this->alphatest ? ALPHA_TEST : 0) |
			(this->normal_map ? NORMAL_MAP : 0) |
			(this->diffuse.length() ? DIFFUSE_MAP : 0) |
			(this->skinned ? SKINNING : 0) |
			0;// (POINT_LIGHTS);

		if (r.shadows_)
			id |= SHADOWS;

		//supply the surface shader here and have it pick from the right list based on that
		//these are shaders that will be used depending on lighting and scene configurations
		this->shader_ptr = shaders->GetShader(id, this);// this->defines, this->surface_shader);
		this->shader_lit_ptr = shaders->GetShader(id | POINT_LIGHTS, this);
		this->shader_unskinned_ptr = shaders->GetShader(id & (~SKINNING), this);
		this->shader_lit_unskinned_ptr = shaders->GetShader((id | POINT_LIGHTS) & (~SKINNING), this);

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

bool read_bool(const std::string& in)
{
	if (in == "false")
		return false;
	return true;
}

//loads a material from a .mat file

//new idea
//todo: shader stage passes
//can have a stage for each kind of shader stuff, so can make it easy to add things like fog to all shaders
//also can be used for shadows and lighting
IMaterial* IMaterial::load_as_resource(const std::string &path, IMaterial* res)
{
	//	and fix branch culling / lighting issues
	auto mat = res;

	std::string name = path;
	int offset = name.find_last_of('/') +1;
	if (offset == 0)
		offset = name.find_last_of('\\')+1;
	
	name = name.substr(offset, name.length() - offset);

	//need to preserve children
	auto old = mat->children;

	new (mat)IMaterial(name.c_str());

	//restore children
	mat->children = old;

	//setup defaults here
	mat->SetDefaults();

	//this name needs to be more readily changed and not buried in code
	mat->shader_name = JET_SHADER_FOLDER;
	mat->shader_name += JET_DEFAULT_SHADERBUILDER;
	mat->shader_builder = true;
	mat->skinned = true;//this doesnt always need to be true

	std::string realname = JET_CONTENT_FOLDER;
	realname += name;
	
	auto file = std::ifstream(realname, std::ios::binary /*| std::ios::ate*/);
	if (!file)
	{
		printf("ERROR: Could not load material '%s'\n", realname.c_str());
	}
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string a, b;
		if (!(iss >> a/* >> b*/)) {
			break;
		} // error

		iss >> b;

		// process pair (a,b)
		if (a == "normal:")
			mat->normal = b;
		else if (a == "diffuse:")
			mat->diffuse = b;
		else if (a == "alpha:")
			mat->alpha = read_bool(b);
		else if (a == "alpha_test:")
			mat->alphatest = read_bool(b);
		else if (a == "include:")
		{
			mat->base_material = b;
			//ok, todo lets include this material information

			auto base_mat = resources.get_unsafe<IMaterial>(b);
			if (std::find(base_mat->children.begin(), base_mat->children.end(), mat) == base_mat->children.end())
				base_mat->children.push_back(mat);

			//ok, now inherit information from it
			mat->surface_shader = base_mat->surface_shader;
			mat->diffuse = base_mat->diffuse;
			mat->alpha = base_mat->alpha;
			mat->alphatest = base_mat->alphatest;
			mat->normal = base_mat->normal;
		}
		else if (a == "cull:")
		{
			if (!read_bool(b))
				mat->cullmode = CULL_NONE;
		}
		else if (a == "surface:")
		{
			int start = file.tellg();
			file.seekg(0, file.end);
			int length = file.tellg();
			length -= start;
			file.seekg(start, file.beg);

			if (length == 0)
				continue;

			char * buffer = new char[length + 1];

			// read data as a block:
			file.read(buffer, length);
			buffer[length] = 0;
			//parse in the rest of the lines as the surface shader

			res->surface_shader = buffer;
			
			//dooo itt
			delete[] buffer;
			//surface shader is only for shaderbuilder stuff
		}
		//else if (a == "cast_shadows:")
		//	mat->
	}

	//force update of children
	for (int i = 0; i < res->children.size(); i++)
	{
		IMaterial::load_as_resource(JET_CONTENT_FOLDER+res->children[i]->name, res->children[i]);
	}
	if (res->shader_builder)
	{
		auto shaders = resources.get_unsafe<ShaderBuilder>(res->shader_name);
		shaders->InvalidateCache(res);
	}
	//figure out how to put the surface shader into the material
	return mat;
}


void IMaterial::SetDefine(const std::string& name, const std::string& value)
{
	if (this->shader_builder == false)//this is only for shader builders
		throw 7;

	this->defines[name] = value;

	auto shaders = resources.get_unsafe<ShaderBuilder>(this->shader_name);

	//apply the changes
	shaders->InvalidateCache(this);
}

void IMaterial::SetDynamicDiffuseTexture(CTexture* tex)
{
	this->diffuse = "DYNAMIC";
	this->texture = tex->texture_rv;
}