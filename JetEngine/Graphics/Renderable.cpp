#include "Renderable.h"
//#include "Models\ObjModel.h"
//#include "../ModelData.h"
//#include "Renderer.h"
#include "../IMaterial.h"
#include "CVertexBuffer.h"
//#include "RenderTexture.h"

IMaterial chunk_material("chunk", "Shaders/world.shdr", Point, "texture.png", CULL_CW, false);
IMaterial achunk_material("alphachunk", "Shaders/world.shdr", Point, "texture.png", CULL_CW, true);


//#include <JetContext.h>
//#pragma comment (lib, "JetLang.lib")

//#include <istream>
//#include <fstream>
//#include <algorithm>

//#include "../Util/Profile.h"


void ChunkRenderable::Render(CCamera* cam, std::vector<RenderCommand>* queue)//void Render(CRenderer* render)
{
	if (this->vb->GetSize() > 0)
	{
		RenderCommand rc;
		rc.material_instance.extra = 0;
		rc.material = &chunk_material;
		rc.alpha = false;
		//rc.shader = 1;
		rc.dist = this->dist;
		rc.source = this;
		rc.mesh.ib = 0;
		rc.mesh.vb = this->vb;
		rc.mesh.primitives = this->vb->GetSize() / this->vb->GetStride();
		queue->push_back(rc);
	}

	if (this->water_vb->GetSize() > 0)
	{
		RenderCommand rc;
		rc.material = &achunk_material;
		rc.alpha = true;
		rc.material_instance.extra = 0;
		//rc.shader = 1;
		rc.dist = this->dist;
		rc.source = this;
		rc.mesh.ib = 0;
		rc.mesh.vb = this->water_vb;
		rc.mesh.primitives = this->water_vb->GetSize() / this->water_vb->GetStride();
		queue->push_back(rc);
	}
}
