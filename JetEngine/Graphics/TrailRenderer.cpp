#include "TrailRenderer.h"

//if I get more trails than 40, just spawn another one of these, maybe

TrailRenderer::TrailRenderer()
{
	int stride = 100;
	this->vb.Allocate(num_trails*trail_size*stride, stride);
}


TrailRenderer::~TrailRenderer()
{
}

int TrailRenderer::AllocateTrail()
{
	todo
}
add missile trails
void TrailRenderer::FreeTrail(int id)
{

}

void TrailRenderer::AppendToTrail(int id, const Vec3& position, const Vec3& tangent)
{

}

void TrailRenderer::Render()
{

}