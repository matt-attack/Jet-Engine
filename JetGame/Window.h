#pragma once

struct HINSTANCE__;
struct HWND__;

class CGame;
class Window
{
	HWND__* hWnd;
public:
	Window(HINSTANCE__* instance, int nCmdShow, int xRes, int yRes);
	~Window();

	void AddGame(CGame* game);

	void* GetOSHandle()
	{
		return hWnd;
	}

	void ProcessMessages();
};