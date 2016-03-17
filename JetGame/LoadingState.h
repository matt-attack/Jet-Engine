
#ifndef LOADINGSTATE_HEADER
#define LOADINGSTATE_HEADER

#include "CGameState.h"
#include <JetEngine/gui/gui_window.h>
#include <JetEngine/gui/gui_progressbar.h>
#include <JetEngine/Util/CThread.h>


class CLoadingState;

struct LoadingThreadData
{
	int finished;
	CGameState* state_to_load;
	CLoadingState* loading;
	float percent;
	char* status;
};

THREAD_RETURN LoadingFunction(THREAD_INPUT data);

class CLoadingState : public CGameState
{
	gui_window desktop;
	gui_progressbar progress;

	CGameState* state_to_load;
	CGameState* previous_state;

	LoadingThreadData* thread_data;
	CThread thread;
public:
	CLoadingState(void);
	~CLoadingState(void);

	virtual void Init(CGame* game);

	virtual void Cleanup() {};

	virtual bool Load(char** c, float* f) { return true; };//called when loading the game by CLoadingState

	virtual void Pause() {};
	virtual void Resume() {};

	virtual void MouseEvent(CGame* game, int x, int y, int eventId) {};
	virtual void KeyboardEvent(CGame* game, int eventType, int keyId) {};

	virtual void HandleEvents(CGame* game, int messagetype, void* data1, void* data2) {};
	virtual void Update(CGame* game, float dTime);
	virtual void Draw(CGame* game, float dTime);


	void SetStateToLoad(CGameState* state)
	{
		this->state_to_load = state;
	};

	void SetPreviousState(CGameState* state)
	{
		this->previous_state = state;
	}

	void AbortLoad()
	{
		this->thread_data->finished = true;
	}
	void SetProgress(float p)
	{
		this->progress.setProgress(p);
	}
	void SetStatus(char* str)
	{
		this->progress.settext(str);
	}
};
#endif

