#include "LoadingState.h"
#include <JetEngine/Graphics/CRenderer.h>

extern CRenderer* renderer;
THREAD_RETURN LoadingFunction(THREAD_INPUT data)
{
	LoadingThreadData* d = (LoadingThreadData*)data;
	d->finished = 0;
	bool success = d->state_to_load->Load(&d->status, &d->percent);
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
};

CLoadingState::~CLoadingState(void)
{
	free(this->thread_data);
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
};

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
	progress.setpos(renderer->xres/2 - 200,renderer->yres/2 - 25);
	
	this->SetProgress(this->thread_data->percent);
	this->SetStatus(this->thread_data->status);
	renderer->Clear(1.0f, 1.0f, 1.0f, 1.0f);
	desktop.renderall(0,0,0,0,1);
};