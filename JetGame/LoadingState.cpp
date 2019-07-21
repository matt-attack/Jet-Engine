#include "LoadingState.h"
#include <JetEngine/Graphics/CRenderer.h>
#include <JetEngine/Graphics/VRRenderer.h>
#include <JetEngine/camera.h>
#include <JetEngine/Graphics/RenderTexture.h>
#include <JetEngine/Graphics/Renderer.h>
#include <JetEngine/IMaterial.h>


extern CRenderer* renderer;
THREAD_RETURN LoadingFunction(THREAD_INPUT data)
{
	LoadingThreadData* d = (LoadingThreadData*)data;
	d->finished = 0;
	bool success = d->state_to_load->Load(&d->status, &d->percent);
	//Sleep(30000);//for testing
	if (!success)
	{
		//go back to previous state
		d->finished = -1;
	}
	else
		d->finished = 1;
	return 0;
}

CLoadingState::CLoadingState(void)
{
	desktop.setpos(0,0);
	desktop.setsize(1000,1000);

	progress.setProgress(0.5f);
	progress.settext("hey");
	desktop.AddWindow(&progress);

	this->thread_data = (LoadingThreadData*)malloc(sizeof(LoadingThreadData));
	this->thread_data->finished = 0;
	this->thread_data->state_to_load = state_to_load;
	this->thread_data->loading = this;
	this->thread_data->percent = 0.00f;
	this->thread_data->status = "Loading...";

	this->previous_state = 0;

	this->gui_texture = 0;
}

CLoadingState::~CLoadingState(void)
{
	free(this->thread_data);

	delete this->gui_texture;
}

void CLoadingState::Init(CGame* game)
{
	progress.setpos(renderer->xres/2 - 200,renderer->yres/2 - 25);
	progress.setsize(400, 50);

	this->thread_data->finished = 0;
	this->thread_data->state_to_load = state_to_load;
	this->thread_data->loading = this;
	this->thread_data->percent = 0.00f;
	this->thread_data->status = "Loading...";

	//create loading thread
	thread.Start(&LoadingFunction, this->thread_data);

	// Make a basic model for the floor and gui so we can render it properly with shadows and shizzle
	std::vector<BasicRenderable::EzVert> verts;
	floor.AddRect(verts, Vec3(-100, 0, 0), Vec3(100, 0, 0), Vec3(0, 0, 100));
	floor.SetMeshEasy("whatever", "grid.jpeg", verts.data(), verts.size());

	std::vector<BasicRenderable::EzVert> vertsg;
	gui.AddRect(vertsg, Vec3(-0.5, 1.0, 0), Vec3(1, 0, 0), Vec3(0, 0.5, 0));
	gui.SetMeshEasy("whatever", "", vertsg.data(), vertsg.size());

	gui_texture = CRenderTexture::Create(400, 50, DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
}

void CLoadingState::Update(CGame* game, float dTime)
{
	if (this->thread_data->finished == 1)
	{
		game->ChangeState(this->state_to_load);
		this->thread_data->finished = 0;
	}
	else if (this->thread_data->finished == -1 && this->previous_state)
	{
		game->ChangeState(this->previous_state);
		this->thread_data->finished = 0;
	}
}

void CLoadingState::Draw(CGame* game, float dTime)
{
	game->SetClearColor(1.0, 1.0, 1.0, 1.0);
	r.add_queue_.push_back([this]() {
		progress.setpos(renderer->xres / 2 - 200, renderer->yres / 2 - 25);

		this->SetProgress(this->thread_data->percent);
		this->SetStatus(this->thread_data->status);

		desktop.renderall(0, 0, 0, 0, 1);
	});
	return;
	progress.setpos(renderer->xres/2 - 200,renderer->yres/2 - 25);
	
	this->SetProgress(this->thread_data->percent);
	this->SetStatus(this->thread_data->status);
	renderer->Clear(1.0f, 1.0f, 1.0f, 1.0f);
	desktop.renderall(0,0,0,0,1);

	VRRenderer* vr = dynamic_cast<VRRenderer*>(renderer);
	if (vr)
	{
		CRenderTexture ot = renderer->GetRenderTarget(0);

		//render the progress bar to texture
		renderer->SetRenderTarget(0, this->gui_texture);
		renderer->SetViewport(50, 200);
		progress.setpos(0, 0);
		desktop.renderall(0, 0, 0, 0, 1);

		renderer->SetRenderTarget(0, &ot);
		gui.material->SetDynamicDiffuseTexture(this->gui_texture);

		//ok, lets add a mouselook/kb test vr mode and get this working in it

		vr->Clear(1, 0, 0, 0.1);

		r.add_renderables_.push_back(&floor);
		r.add_renderables_.push_back(&gui);
		//r.AddRenderable(&floor);
		//r.AddRenderable(&gui);

		CCamera lc, rc;
		Matrix4 hmd = vr->GetHMDPose();

		Matrix4 eye;
		vr->GetLeftEyeVMatrix(eye);

		//Model * View * Projection the sequence is Model * View * Eye^-1 * Projection.  
		lc._matrix = lc._matrix*hmd*eye;// this->local_players[0]->cam._matrix *hmd;// hmd * eye;
		vr->GetLeftEyePMatrix(lc._projectionMatrix);

		vr->GetRightEyeVMatrix(eye);
		rc._matrix = rc._matrix*hmd*eye; //rc._matrix*(hmd*eye.Inverse());// this->local_players[0]->cam._matrix *hmd;// hmd * eye;
		vr->GetRightEyePMatrix(rc._projectionMatrix);

		vr->BindEye(Left_Eye);
		//r.Render(&lc, vr);

		vr->BindEye(Right_Eye);
		//r.Render(&rc, vr);

		//r.Finish();
		renderer->SetRenderTarget(0, &ot);
	}

	 /*Also have a VR mode, render progress bar in a room

		 So for this lets have a dark bluish sky with a gridded plane on the floor
		 then can draw the loading stuff in front of us on a plane (lets use a render texture for this one
		 so we can reuse the gui element)*/

	
}