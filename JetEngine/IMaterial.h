#ifndef _MATERIAL_HEADER
#define _MATERIAL_HEADER

#include "Graphics\CRenderer.h"

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
class IMaterial
{
	friend class Renderer;

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

	bool shader_builder = false;
	bool skinned = false;

	IMaterial(char* name);

	//IMaterial(char* name, int shader, FilterMode fmode, char* diffuse, CullMode cmode, bool alpha, bool weaponhack = false);

	IMaterial(char* name, char* shader, FilterMode fmode, char* diffuse, CullMode cmode, bool alpha, bool weaponhack = false);

	//IMaterial(char* name, char* shader, bool shader_builder, FilterMode fmode, char* diffuse, CullMode cmode, bool alpha, bool weaponhack = false);

	//IMaterial(char* name, int shader, FilterMode fmode, char* diffuse, char* normal, CullMode cmode, bool alpha, bool weaponhack = false);

	virtual void Apply(CRenderer* renderer);

	void ApplyShader(bool skinned, bool lit);

	//updates internal data, called when a resource changes
	//or when settings are changed
	//this should load the correct shader pointer
	virtual void Update(CRenderer* renderer);

	static std::map<std::string, IMaterial*>& GetList()
	{
		static std::map<std::string, IMaterial*> materials;
		return materials;
	}

	//loads a material from a .mat file
	static IMaterial* Load(const char* name);
};


//ok, todo: SURFACE SHADERS

//a shader will be made up of a few components
//position / normal generator + tangent system(optional) + surface shader + lighting + shadow
class DamageableMaterial : public IMaterial
{
public:
	CTexture* damage_texture;

	virtual void Apply(CRenderer* renderer)
	{
		IMaterial::Apply(renderer);

		renderer->SetPixelTexture(9, damage_texture);
	}
};
#endif