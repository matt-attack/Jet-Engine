#include "SplashScreen.h"
#include <JetEngine/Graphics/CRenderer.h>
#include <JetEngine/Graphics/Renderer.h>

SplashScreenState::SplashScreenState(void)
{
	desktop.setpos(0, 0);
	desktop.setsize(1000, 1000);

	this->next_state = 0;
	this->time = 0;
}

SplashScreenState::~SplashScreenState(void)
{
	
}

void SplashScreenState::Update(CGame* game, float dTime)
{
	this->time += dTime;

	if (this->time >= 5)
	{
		game->ChangeState(this->next_state);
	}
}

void SplashScreenState::Draw(CGame* game, float dTime)
{
	r.add_queue_.push_back([this]()
	{
		renderer->Clear(1.0f, 0.0f, 0.0f, 0.0f);

		renderer->DrawCenteredText(Rect(0, renderer->yres, 0, renderer->xres), "Powered by Jet Engine", COLOR_ARGB(255, 255, 255, 255));

		desktop.renderall(0, 0, 0, 0, 1);
	});
}