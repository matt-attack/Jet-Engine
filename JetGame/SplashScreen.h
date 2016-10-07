
#ifndef LOADINGSTATE_HEADER
#define LOADINGSTATE_HEADER

#include "CGameState.h"
#include <JetEngine/gui/gui_window.h>
#include <JetEngine/gui/gui_progressbar.h>
#include <JetEngine/Util/CThread.h>

class SplashScreenState : public CGameState
{
	gui_window desktop;
	//gui_progressbar progress;

	CGameState* next_state;

	float time;

public:
	SplashScreenState(void);
	~SplashScreenState(void);

	virtual void Init(CGame* game);

	virtual void Cleanup() {};

	virtual bool Load(char** c, float* f) { return true; };//called when loading the game by CLoadingState

	virtual void Pause() {};
	virtual void Resume() {};

	virtual void MouseEvent(CGame* game, int x, int y, int eventId) { this->time = 1000; };
	virtual void KeyboardEvent(CGame* game, int eventType, int keyId) { this->time = 1000; };

	virtual void HandleEvents(CGame* game, int messagetype, void* data1, void* data2) {};
	virtual void Update(CGame* game, float dTime);
	virtual void Draw(CGame* game, float dTime);


	void SetNextState(CGameState* state)
	{
		this->next_state = state;
	};

	void SetProgress(float p)
	{
		//this->progress.setProgress(p);
	}
	void SetStatus(char* str)
	{
		//this->progress.settext(str);
	}
};
#endif