#include "Renderable.h"
#include "../IMaterial.h"
#include "CVertexBuffer.h"


void BasicRenderable::SetMeshEasy(const std::string& material_name, const std::string& image_name, const EzVert* vertex, int count)
{
	auto mat = new IMaterial(material_name.c_str());
	mat->alpha = false;
	//ok, this is eww, but fine for now
	std::string mname = image_name;
	if (mname.length() > 0 && mname[mname.length() - 1] != 'g')
		mname = mname + ".png";//.tga

	//need to change this to not always be set to skinned, for example for trees
	mat->skinned = false;
	mat->shader_name = "Shaders/ubershader.txt";
	mat->shader_builder = true;

	//temporary test values
	//mat->normal = "brick.jpg";
	//mat->alphatest = true;

	mat->diffuse = mname;
	mat->Update(renderer);//load any associated textures
	material = mat;

	my_vb = CVertexBuffer(VertexBufferUsage::Static);
	VertexElement elm7[] = { { ELEMENT_FLOAT3, USAGE_POSITION },
	{ ELEMENT_FLOAT3, USAGE_NORMAL },
	{ ELEMENT_FLOAT3, USAGE_TANGENT },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };

	my_vb.SetVertexDeclaration(renderer->GetVertexDeclaration(elm7, 4));
	my_vb.Data(vertex, count * sizeof(EzVert), sizeof(EzVert));

	vcount = count;
	vb = &my_vb;
}