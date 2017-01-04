// defines
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DIRECTINPUT_VERSION  0x0800

#include <Windows.h>

// include the basic windows header files and the Direct3D header file
#include <dinput.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "d3dx10.lib")
#pragma comment (lib, "OpenAL32.lib")
#pragma comment (lib, "XInput.lib")

#pragma comment (lib, "JetEngine.lib")
#pragma comment (lib, "JetGame.lib")
//#pragma comment (lib, "JetNet.lib")

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

//#include "Game.h"
#include <JetGame/CGame.h>
#include <JetGame/SplashScreen.h>
#include <JetEngine/Graphics/CRenderer.h>
#include <JetGame/Window.h>

//my globals
CRenderer* renderer;//this global is required to use Jet Engine. It wraps the low level graphics functions provided by DirectX (and once, but coming again OpenGL)

//create your game 
class MyGame : public CGame
{

public:
	void onInit()
	{
		//anything you want run when your game starts
	}

	void onUpdate()
	{
		//anything you want run once per frame before rendering
	}
};

//your gamestate
class MyGameState : public CGameState
{
	gui_window desktop;

public:
	MyGameState(void)
	{

	}

	~MyGameState(void)
	{

	}

	//all the functions you can implement in your gamestate
	virtual void Init(CGame* game) {}

	virtual void Cleanup() {};

	virtual bool Load(char** c, float* f) { return true; };//called when loading the game by CLoadingState

	virtual void Pause() {};
	virtual void Resume() {};

	virtual void MouseEvent(CGame* game, int x, int y, int eventId) {  };
	virtual void KeyboardEvent(CGame* game, int eventType, int keyId) {  };

	virtual void HandleEvents(CGame* game, int messagetype, void* data1, void* data2) {};
	virtual void Update(CGame* game, float dTime) {}
	virtual void Draw(CGame* game, float dTime)
	{
		renderer->Clear(1, 1, 0, 0);

		renderer->DrawText(0, 50, "Hello", 0xFFFFFFFF);
	}
};


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//allocate my game and the renderer
	auto game = new MyGame;

	renderer = new CRenderer();

	Window w(hInstance, nCmdShow, 700, 700);
	w.AddGame(game);

	//initialize the renderer to work with the window and have a default size
	renderer->Init((HWND)w.GetOSHandle(), 700, 700);

	//initialize the game with the window as well
	game->Init(&w);

	//create the splash screen and have it set the next state as MyGameState
	auto splash = new SplashScreenState;
	splash->SetNextState(new MyGameState);
	game->PushState(splash);//this pushes it onto the stack of gamestates so it is rendered and updated

	while (game->Running())
	{
		game->Update();

		if (renderer->Vsync() == false)
			Sleep(1);//this prevents the game from using 100% CPU and limits framerate slightly
		else
			Sleep(0);//this gives other programs a chance to run if need be
	}

	game->Cleanup();

	resources.release_unused();//remove resources before killing renderer

	delete renderer;
	delete game;

	return 0;
}