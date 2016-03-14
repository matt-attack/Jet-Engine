#include "Window.h"

#include <map>

#include <Windows.h>
#include "CGame.h"

std::map<HWND, CGame*> games;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}

Window::Window(HINSTANCE hInstance, int nCmdShow, int xRes, int yRes)
{
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	int xres = xRes; int yres = yRes;
	int dx, dy;
	GetDesktopResolution(dx, dy);
	if (xres > dx)
		xres = dx;
	if (yres > dy - 65)//take into account taskbar and stuff
		yres = dy - 65;

	hWnd = CreateWindowEx(NULL,
		L"WindowClass",
		L"Mechstravaganza",
		WS_OVERLAPPEDWINDOW,
		0, 0,
		xres, yres + 30,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	//fuddle with window size
	RECT r;
	GetWindowRect(hWnd, &r);
	SetWindowPos(hWnd, 0, 0, 0, abs(r.left - r.right) + 1, abs(r.top - r.bottom), 0);
	SetWindowPos(hWnd, 0, 0, 0, abs(r.left - r.right), abs(r.top - r.bottom), 0);
}

Window::~Window()
{
	games[hWnd] = 0;
}

void Window::AddGame(CGame* game)
{
	games[hWnd] = game;
}

void Window::ProcessMessages()
{
	MSG msg;
	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

bool last = true;
void SetShowCursor(bool a)
{
	if (a != last)
	{
		last = a;
		ShowCursor(a);
	}
}

bool hasfocus = false;
bool AIn = true;
int SelectedBlockType = 1;
// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto game = games[hWnd];
	if (game == 0)
		return DefWindowProc(hWnd, message, wParam, lParam);

	auto input = game->GetInput();
	switch (message)
	{
		///ADD KEY UP AND KEY DOWN
	case WM_KEYUP:
	{
		game->KeyboardEvent(ENG_KEY_UP, wParam);
		break;
	}
	case WM_KEYDOWN:
	{
		//exclude repeats in keydown
		if ((lParam & 1 << 30) <= 1)//bit 30
			game->KeyboardEvent(ENG_KEY_DOWN, wParam);

		break;
	}
	case WM_CHAR:
	{
		game->KeyboardEvent(ENG_CHAR, wParam);

		break;
	}
	case WM_RBUTTONDOWN:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_R_DOWN);

		break;
	}
	case WM_RBUTTONUP:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_R_UP);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_L_DOWN);

		break;
	}
	case WM_LBUTTONUP:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_L_UP);

		break;
	}
	case WM_SIZE:
	{
		if (renderer)
			renderer->Resize((int)LOWORD(lParam), (int)HIWORD(lParam));

		break;
	}
	case WM_MOUSEMOVE: //not a good method
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);
		if (input->lmouse_down == true)
		{
			//drag
			game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_L_DRAG);
		}
		break;
	}
	/*case WM_MOUSEWHEEL:
	{
	OutputDebugString("mousewheel\n");
	break;
	}*/
	case WM_INPUT://get mouse input
	{
		if (AIn)
		{
			UINT dwSize = 40;
			static BYTE lpd[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpd, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpd;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int deltaX = raw->data.mouse.lLastX;
				int deltaY = raw->data.mouse.lLastY;
				input->deltaX = deltaX;
				input->deltaY = deltaY;

				bool lButtonDown = raw->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN;
				bool rButtonDown = raw->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN;
				input->left_mouse = lButtonDown;
				input->right_mouse = rButtonDown;

				if (raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL)
				{
					USHORT scroll = raw->data.mouse.usButtonData;
					if (scroll == 65416)
					{
						//if (SelectedBlockType > 0)
						//{
						SelectedBlockType++;
						//}
					}
					else if (scroll > 0)
					{
						if (SelectedBlockType > 0)
							SelectedBlockType--;
					}
					//if (gMultiplayer->player)
					//gMultiplayer->player->selectedslot = SelectedBlockType%9;
				}

				//lastl = lButtonDown;
				//lastr = rButtonDown;
			}
		}

		break;
	}
	case WM_SETFOCUS:
	{
		AIn = true;//we have focus, accept input
		hasfocus = true;
		break;
	}
	case WM_KILLFOCUS:
	{
		SetShowCursor(true);
		AIn = false;//no focus, no input accepted
		hasfocus = false;
		game->Pause();
		break;
	}
	case WM_DESTROY:
	{
		//game.Quit();
		PostQuitMessage(0);
		//keepalive = false;
		game->Quit();

		return 0;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

