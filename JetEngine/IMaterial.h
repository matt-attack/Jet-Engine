#ifndef _MATERIAL_HEADER
#define _MATERIAL_HEADER

#include "Graphics\CRenderer.h"
#include "ResourceManager.h"

#include <map>

class IMaterial;

enum ShaderFeatures
{
	SKINNING = 1,
	NORMAL_MAP = 2,
	POINT_LIGHTS = 4,
	SHADOWS = 8,
	ALPHA_TEST = 16,
};

//each mesh will have one of these
class IMaterial: public Resource
{
	friend class Renderer;
	friend class ResourceManager;
public:
	int key;//generated from properties of material, used for sorting
	bool alpha;
	bool alphatest;
	bool depthhack;//use this for first person weapons views
private:
	bool needs_tangent = false;
public:
	FilterMode filter;
	Texture texture;
	Texture normal_map;

	CullMode cullmode;

	//this is stupid, but oh well
	CShader* shader_ptr;//use this instead
	CShader* shader_lit_ptr = 0;
	CShader* shader_lit_unskinned_ptr = 0;
	CShader* shader_unskinned_ptr = 0;

	//not used during rendering
	std::string name;
	std::string diffuse;
	std::string normal;
	std::string shader_name;

	std::string surface_shader;//if we want a custom one

	std::string base_material;//what we include from
	std::vector<IMaterial*> children;//list of materials that include from us

private:
	//custom defines/settings for shaders
	//when this changes need to invalidate shaders and regenerate
	std::map<std::string, std::string> defines;
public:

	bool shader_builder = false;
	bool skinned = false;
private:
	IMaterial() {  };
public:
	IMaterial(const char* name);
	~IMaterial();

	
	IMaterial(char* name, char* shader, FilterMode fmode, char* diffuse, CullMode cmode, bool alpha, bool weaponhack = false);

	//sets a shader define value, only applies to shader builder type materials
	void SetDefine(const std::string& name, const std::string& value);

	//this applies material properties (blend states, textures) that are constant across all models with this material
	virtual void Apply(CRenderer* renderer);

	//this selects the proper shader for rendering depending on the models run time condition
	void ApplyShader(bool skinned, int lights);

	//updates internal data, called when a resource changes
	//or when settings are changed
	//this should load the correct shader pointer
	virtual void Update(CRenderer* renderer);


	//gets the list of materials currently created organized by name
	static std::map<std::string, IMaterial*>& GetList()
	{
		static std::map<std::string, IMaterial*> materials;
		return materials;
	}

	//resource stuff
	virtual void Reload(ResourceManager* mgr, const std::string& filename) 
	{
		this->load_as_resource(filename, this);
	};
	static IMaterial* load_as_resource(const std::string &path, IMaterial* res);
};


//ok, todo: SURFACE SHADERS

//a shader will be made up of a few components
//position / normal generator + tangent system(optional) + surface shader + lighting + shadow
#endif