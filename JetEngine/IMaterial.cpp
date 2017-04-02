#include "IMaterial.h"
#include "Graphics/CTexture.h"
#include "Graphics/Shader.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderable.h"
#include "ResourceManager.h"

#include <fstream>

std::map<std::string, IMaterial*> materials;

const int num_builder_options = 5;
class ShaderBuilder : public Resource
{
	std::string path;

public:

	std::map<int, CShader*[1 << num_builder_options]> shaders;
	//CShader* shaders[1 << num_builder_options];

private:
	std::map<std::string, std::string> defines;//set whenever you call LoadShader with those defines
public:

	ShaderBuilder()
	{
		
	}

	~ShaderBuilder()
	{
		for (int i = 0; i < (1 << num_builder_options); i++)
			delete shaders[i];
	}

	virtual void Reload(ResourceManager* mgr, const std::string& path)
	{
		if (this->path.length())
		{
			for (auto& shader : this->shaders)
				for (int i = 0; i < (1 << num_builder_options); i++)
					if (shader.second[i])
					{
						delete shader.second[i];
						shader.second[i] = 0;// this->LoadShader(i, this->defines, this->surface_shader);
					}

			return;
		}
		this->path = path;
	}

	void InvalidateCache()
	{
		for (auto& shader : this->shaders)
			for (int i = 0; i < (1 << num_builder_options); i++)
				if (shader.second[i])
				{
					delete shader.second[i];
					shader.second[i] = 0;
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

	CShader* GetShader(int id, std::map<std::string, std::string>& defines, std::string& surface_shader)
	{
		//get a key
		int key = surface_shader.length();
		auto res = shaders.find(key);
		if (res != shaders.end() && res->second[id])
			return res->second[id];

		this->shaders[key][id] = this->LoadShader(id, defines, surface_shader);

		return this->shaders[key][id];
	}

private:
	std::string surface_shader;
	CShader* LoadShader(int id, std::map<std::string, std::string>& defines, std::string& surface_shader)
	{
		this->surface_shader = surface_shader;
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

		//dont do this, need to new it or destruct then placement new
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
		
		if (add_dummy)
		{
			//add dummy surface shader
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

		//need shaderbuilder to hold a bunch of shaders, one for each surface shader
		//these are static material values
		int id = (this->alphatest ? ALPHA_TEST : 0) |
			(this->normal_map ? NORMAL_MAP : 0) |
			(this->skinned ? SKINNING : 0) |
			0;// (POINT_LIGHTS);

		if (r._shadows)
			id |= SHADOWS;

		//supply the surface shader here and have it pick from the right list based on that
		//these are shaders that will be used depending on lighting and scene configurations
		this->shader_ptr = shaders->GetShader(id, this->defines, this->surface_shader);
		this->shader_lit_ptr = shaders->GetShader(id | POINT_LIGHTS, this->defines, this->surface_shader);
		this->shader_unskinned_ptr = shaders->GetShader(id & (~SKINNING), this->defines, this->surface_shader);
		this->shader_lit_unskinned_ptr = shaders->GetShader((id | POINT_LIGHTS) & (~SKINNING), this->defines, this->surface_shader);

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
#include <direct.h>
std::string get_working_path()
{
	char temp[512];
	return (getcwd(temp, 512) ? std::string(temp) : std::string(""));
}

#include <Windows.h>

std::vector<std::string> get_all_files_names_within_folder(std::string folder)
{
	std::vector<std::string> names;
	std::string search_path = folder + "*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}
//new idea
//todo: shader stage passes
//can have a stage for each kind of shader stuff, so can make it easy to add things like fog to all shaders
//also can be used for shadows and lighting
#include <iostream>
IMaterial* IMaterial::load_as_resource(const std::string &path, IMaterial* res)//Load(const char* name)
{
	//ok, lets get auto material reloading
	//	add alpha test to shadows
	//	and fix branch culling / lighting issues
	auto mat = res;// new IMaterial((char*)name);

	std::string name = path;
	int offset = name.find_last_of('/') +1;
	if (offset == 0)
		offset = name.find_last_of('\\')+1;
	auto pathh = get_working_path();
	auto filess = get_all_files_names_within_folder("Content/");
	name = name.substr(offset, name.length() - offset);

	//need to preserve children
	auto old = mat->children;

	new (mat)IMaterial(name.c_str());

	//restore children

	mat->children = old;

	//setup defaults here
	mat->alpha = false;
	mat->alphatest = false;
	mat->filter = FilterMode::Linear;
	mat->depthhack = false;
	mat->cullmode = CULL_CW;

	mat->shader_name = "Shaders/ubershader.txt";
	mat->shader_builder = true;
	mat->skinned = true;//this doesnt always need to be true

	std::string realname = "Content/";
	realname += name;
	//realname += ".mat";
	//FILE* f = fopen(realname.c_str(), "rb");
	auto file = std::ifstream(realname, std::ios::binary /*| std::ios::ate*/);
	//char a = file.get();
	std::string line;
	//std::cout << "Error code: " << strerror(errno);
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
		IMaterial::load_as_resource("Content/"+res->children[i]->name, res->children[i]);
	}

	//figure out how to put the surface shader into the material
	return mat;
}


void IMaterial::SetDefine(const std::string& name, const std::string& value)
{
	this->defines[name] = value;

	auto shaders = resources.get_unsafe<ShaderBuilder>(this->shader_name);

	//apply the changes
	shaders->InvalidateCache();
}