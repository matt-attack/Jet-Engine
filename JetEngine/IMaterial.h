#ifndef _MATERIAL_HEADER
#define _MATERIAL_HEADER

#include "Graphics\CRenderer.h"

#include <map>

class IMaterial;

//each mesh will have one of these
class IMaterial
{
public:
	int key;//generated from properties of material, used for sorting
	bool alpha;
	bool alphatest;
	bool depthhack;//use this for first person weapons views
	FilterMode filter;
	Texture texture;
	
	CShader* shader_ptr;//use this instead

	CullMode cullmode;

	//not used during runtime
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

	//updates internal data, called when a resource changes
	//or when settings are changed
	//this should load the correct shader pointer
	virtual void Update(CRenderer* renderer);

	static std::map<std::string, IMaterial*>& GetList()
	{
		static std::map<std::string, IMaterial*> materials;
		return materials;
	}
};

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