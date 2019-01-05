#ifndef _MATERIAL_HEADER
#define _MATERIAL_HEADER

#include "Graphics/CRenderer.h"
#include "ResourceManager.h"

#include <map>

enum ShaderFeatures
{
	SKINNING = 1,
	DIFFUSE_MAP = 2,
	NORMAL_MAP = 4,
	POINT_LIGHTS = 8,
	SHADOWS = 16,
	ALPHA_TEST = 32,
};

//each mesh will have one of these
class IMaterial: public Resource
{
	friend class Renderer;
	friend class ResourceManager;
	friend class ShaderBuilder;
public:
	//User configured material booleans

	bool alpha;//alpha blending
	bool alphatest;//alpha test
	bool depthhack;//use this for first person weapons views
	bool shader_builder = false;//should we use the shaderbuilder/ubershader system for particular shader/material?
	bool skinned = false;//does the model use vertex skinning?

private:
	bool needs_tangent = false;//turned on if there is a normal map

public:
	//User configured material enumerations
	FilterMode filter;
	CullMode cullmode;

private:
	//The actual pointers to the textures used. Loaded by Update()
	Texture texture = 0;
	Texture normal_map = 0;

	// List of textures to possibly be set that are material related todo use this
	//Texture textures[4];

	//not the best way to manage this, but it works
	CShader* shader_ptr;
	CShader* shader_lit_ptr = 0;
	CShader* shader_lit_unskinned_ptr = 0;
	CShader* shader_unskinned_ptr = 0;
public:

	// User configured material option strings
	std::string name;//name of the material
	std::string diffuse;//file name of the diffuse texture
	std::string normal;//file name of the normal map texture
	std::string shader_name;//file name of a shader file to use

	std::string surface_shader;//custom surface shader string
	//the surface shader is of the form
	//void SurfaceShader(inout VS_OUTPUT In) { /*do stuff here*/ }

	//VS_OUTPUT has the following defintion
	/*struct VS_OUTPUT
	{
		float4 Position : SV_Position;
		float4 Diffuse : COLOR0;
		float2 TexCoord : TEXCOORD0;
		float3 WorldPos : TEXCOORD1;
#ifdef SHADOWS
		float Depth : TEXCOORD2;
#endif
		float3 Normal : NORMAL0;
#ifdef NORMAL_MAP
		float3 Tangent : TANGENT0;
		float3 Bitangent : BITANGENT0;
#endif
	};*/

	std::string base_material;//name of the material we include from

private:
	std::vector<IMaterial*> children;//list of materials that include from us

	//custom defines/settings for shaders
	//when this changes need to invalidate shaders and regenerate
	std::map<std::string, std::string> defines;
	
private:
	IMaterial() {  };
public:

	IMaterial(const char* name);
	IMaterial(const char* name, const IMaterial* to_copy);
	IMaterial(char* name, char* shader, FilterMode fmode, char* diffuse, CullMode cmode, bool alpha, bool weaponhack = false);

	virtual ~IMaterial();

	//Sets a shader define value, only applies to shader builder type materials
	void SetDefine(const std::string& name, const std::string& value);

	//This applies material properties (blend states, textures) that are constant across all models with this material
	virtual void Apply(CRenderer* renderer);

	//this selects the proper shader for rendering depending on the models run time condition
	void ApplyShader(bool skinned, int lights);

	//updates internal data, called when a resource changes
	//or when settings are changed
	//also called once a frame in the Renderer before drawing
	//this should load the correct shader pointers
	virtual void Update(CRenderer* renderer);

	// This sets the diffuse texture to a raw one instead of a file
	void SetDynamicDiffuseTexture(CTexture* tex);


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
	}
	static IMaterial* load_as_resource(const std::string &path, IMaterial* res);

private:

	void SetDefaults();
};

//a shader will be made up of a few components
//position / normal generator + tangent system(optional) + surface shader + lighting + shadow
#endif