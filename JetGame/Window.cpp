#include "Window.h"

#include <map>

#include <Windows.h>
#include "CGame.h"

std::map<HWND, std::pair<CGame*, Window*>> games;

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
		L"Jet Engine",
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
	games[hWnd] = { 0, 0};
}

void Window::SetGame(CGame* game)
{
	games[hWnd] = { game, this };
}

bool Window::Destroyed()
{
	return this->destroyed;
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

void Window::SetTitle(const char* title)
{
	SetWindowTextA(this->hWnd, title);
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
#include <hidsdi.h>
bool hasfocus = false;
bool AIn = true;
int SelectedBlockType = 1;
// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto game = games[hWnd].first;
	//if (game == 0)
	//	return DefWindowProc(hWnd, message, wParam, lParam);

	auto window = games[hWnd].second;

	if (window == 0)
		return DefWindowProc(hWnd, message, wParam, lParam);

	auto input = game->GetInput();
	switch (message)
	{
		///ADD KEY UP AND KEY DOWN
	case WM_KEYUP:
	{
		if (game)
			game->KeyboardEvent(ENG_KEY_UP, wParam);
		break;
	}
	case WM_KEYDOWN:
	{
		//exclude repeats in keydown
		if (game && (lParam & 1 << 30) <= 1)//bit 30
			game->KeyboardEvent(ENG_KEY_DOWN, wParam);

		break;
	}
	case WM_CHAR:
	{
		if (game)
			game->KeyboardEvent(ENG_CHAR, wParam);

		break;
	}
	case WM_RBUTTONDOWN:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		if (game)
			game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_R_DOWN);

		break;
	}
	case WM_RBUTTONUP:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		if (game)
			game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_R_UP);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		if (game)
			game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_L_DOWN);

		break;
	}
	case WM_LBUTTONUP:
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);

		if (game)
			game->MouseEvent(input->m_pos.x, input->m_pos.y, ENG_L_UP);

		break;
	}
	case WM_SIZE:
	{
		if (renderer)
			renderer->Resize((int)LOWORD(lParam), (int)HIWORD(lParam));

		//window->x_size = (int)LOWORD(lParam);
		//window->y_size = (int)HIWORD(lParam);
		break;
	}
	case WM_MOUSEMOVE: //not a good method
	{
		GetCursorPos(&input->m_pos);
		ScreenToClient(hWnd, &input->m_pos);
		if (game && input->lmouse_down == true)
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
			static BYTE lpd[80];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpd, &dwSize, sizeof(RAWINPUTHEADER));
			if (dwSize > 80)
				throw 7;
			RAWINPUT* raw = (RAWINPUT*)lpd;

			if (raw->header.dwType == RIM_TYPEHID)
			{
				unsigned int bufferSize;
				GetRawInputDeviceInfo(raw->header.hDevice,
					RIDI_PREPARSEDDATA, NULL, &bufferSize);
				auto pPreparsedData = (PHIDP_PREPARSED_DATA)HeapAlloc(GetProcessHeap(), 0, bufferSize);
				GetRawInputDeviceInfo(raw->header.hDevice,
					RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize);

				HIDP_CAPS Caps;
				HidP_GetCaps(pPreparsedData, &Caps);
				HIDP_BUTTON_CAPS pButtonCaps[3];// = (PHIDP_BUTTON_CAPS)HeapAlloc(GetProcessHeap(), 0, sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps);

				auto capsLength = Caps.NumberInputButtonCaps;
				HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData);
				auto g_NumberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

				HIDP_VALUE_CAPS pValueCaps[10];// = (PHIDP_VALUE_CAPS)HeapAlloc(GetProcessHeap(), 0, sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps);
				capsLength = Caps.NumberInputValueCaps;
				HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData);

				unsigned long usageLength = g_NumberOfButtons;
				USAGE usage[30];// = &pButtonCaps->Range.UsageMin;
				HidP_GetUsages(
					HidP_Input, pButtonCaps->UsagePage, 0, usage,
					&usageLength, pPreparsedData,
					(PCHAR)raw->data.hid.bRawData, raw->data.hid.dwSizeHid
					);

				//read the device
				input->active_joystick = raw->header.hDevice;
				RawDevice& dev = input->devices[raw->header.hDevice];
				dev.num_buttons = g_NumberOfButtons;
				ZeroMemory(dev.buttons, sizeof(bool)*30);
				for (int i = 0; i < usageLength; i++)
					dev.buttons[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

				dev.num_axes = Caps.NumberInputValueCaps;
				for (int i = 0; i < Caps.NumberInputValueCaps; i++)
				{
					unsigned long value;
					HidP_GetUsageValue(
						HidP_Input, pValueCaps[i].UsagePage, 0,
						pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
						(PCHAR)raw->data.hid.bRawData, raw->data.hid.dwSizeHid);

					float range = pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
					if (pValueCaps[i].Range.UsageMin <= 0x32)
						dev.axes[pValueCaps[i].Range.UsageMin - 0x30] = ((float)value - range / 2) / (range / 2);
					else if (pValueCaps[i].Range.UsageMin == 0x35)
						dev.axes[3] = ((float)value - range / 2) / (range / 2);
					else if (pValueCaps[i].Range.UsageMin == 0x39)
						dev.axes[4] = value;
				}

				/*for (int i = 0; i < dev.num_axes; i++)
				{
					if (abs(dev.axes[i]) < 10000)
						printf("%f ", dev.axes[i]);
				}
				printf("\n");*/
				HeapFree(GetProcessHeap(), 0, pPreparsedData);
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE)
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
		PostQuitMessage(0);

		if (game)
			game->Quit();

		if (window)
			window->destroyed = true;

		return 0;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

