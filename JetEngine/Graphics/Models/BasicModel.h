#ifndef BASICMODEL_HEADER
#define BASICMODEL_HEADER

#include "../../ModelData.h"
#include "../Renderable.h"

class CCamera;

class BasicModel: public Renderable
{
	//CVertexBuffer* vb;
	int verts;
public:
#ifdef ANDROID
	GLuint obj;
#endif
	BasicModel();

	virtual ~BasicModel();

	void MakeCubeModel(int blockid, float scale, unsigned long color = 0xFFFFFFFF);
	void MakeFlatModel(int blockid, float scale);

	//this must be implemented by every model type
	virtual void Render(CCamera* cam, std::vector<RenderCommand>* queue);
	//virtual void Render(CRenderer* renderer);
};
#endif