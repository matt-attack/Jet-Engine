#include "SplashScreen.h"
#include <JetEngine/Graphics/CRenderer.h>

SplashScreenState::SplashScreenState(void)
{
	desktop.setpos(0, 0);
	desktop.setsize(1000, 1000);

	//progress.setProgress(0.5f);
	//progress.settext("hey");
	//desktop.AddWindow(&progress);

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
	renderer->Clear(1.0f, 0.0f, 0.0f, 0.0f);

	renderer->DrawCenteredText(Rect(0, renderer->xres, 0, renderer->yres), "Hi im a splash screen", COLOR_ARGB(255, 255, 255, 255));
	//draw the text
	//progress.setpos(renderer->xres / 2 - 200, renderer->yres / 2 - 25);

	desktop.renderall(0, 0, 0, 0, 1);
}