#pragma once

struct HINSTANCE__;
struct HWND__;

class CGame;
class Window
{
	HWND__* hWnd;
public:

	bool destroyed = false;

	Window(HINSTANCE__* instance, int nCmdShow, int xRes, int yRes);
	~Window();

	void SetGame(CGame* game);

	void SetTitle(const char* title);

	void* GetOSHandle()
	{
		return hWnd;
	}

	void ProcessMessages();

	bool Destroyed();
};