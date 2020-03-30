#ifndef BUILDINGMODEL_HEADER
#define BUILDINGMODEL_HEADER

#include "../Renderable.h"

class CCamera;

class BuildingModel : public Renderable
{
	//CVertexBuffer* vb;
	int verts;
public:
	Matrix4 matrix;
#ifdef ANDROID
	GLuint obj;
#endif
	BuildingModel();

	virtual ~BuildingModel();

	void MakeCubeBuilding(float height, float x_size, float y_size, unsigned long color);

	void MakeCubeModel(int blockid, float scale, unsigned long color = 0xFFFFFFFF);
	void MakeFlatModel(int blockid, float scale);

	//this must be implemented by every model type
	virtual void Render(const CCamera* cam, std::vector<RenderCommand>* queue);
	//virtual void Render(CRenderer* renderer);
};
#endif