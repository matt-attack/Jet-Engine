#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "CGame.h"

#define EVENT_TEXT_INPUT 98

class CGameState
{
public:
	virtual void Init(CGame* game) = 0;
	virtual void Cleanup() = 0;

	virtual bool Load(char** status, float* progress) { return true;};//called when loading the game by CLoadingState

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void BindPress(CGame* game, int player, int bind) {};
	virtual void MouseEvent(CGame* game, int x, int y, int eventId) {};
	virtual void KeyboardEvent(CGame* game, int eventType, int keyId) {};
	virtual void TouchEvent(CGame* game, int eventType, float x, float y, float dx, float dy) {};

	virtual void HandleEvents(CGame* game, int messagetype, void* data1, void* data2) = 0;
	virtual void Update(CGame* game, float dTime) = 0;
	virtual void Draw(CGame* game, float dTime) = 0;

	void ChangeState(CGame* game, CGameState* state) {
		game->ChangeState(state);
	}

	virtual ~CGameState() {};

protected:
	CGameState() { }
};

#endif