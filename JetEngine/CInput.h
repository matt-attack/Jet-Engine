
#pragma once

#ifndef _WIN32
struct POINT
{
	int x,y;
};
#else
#include <Windows.h>
#endif
#include <XInput.h>
#include "Defines.h"
#include <vector>
#include <map>

enum KeyIDs
{
	KEY_BACKSPACE = 8,
	KEY_ENTER = 13,
	KEY_SHIFT = 16,
	KEY_ESC = 27,
	KEY_SPACE = 32,
	KEY_DELETE = 46,
	KEY_CONTROL = 0x11,
	KEY_TAB = 0x09,

	KEY_RIGHT = 0x27,
	KEY_LEFT = 0x25,
	KEY_FORWARD = 0x26,//VK_UP,
	KEY_BACK = 0x28,// VK_DOWN,
	/* keys 0-9 are 0x30 thru 0x39 */

	KEY_A = 65,
	KEY_B = 66,
	KEY_C = 67,
	KEY_D = 68,
	KEY_E = 69,
	KEY_F = 70,
	KEY_G = 71,
	KEY_H = 72,
	KEY_I = 73,
	KEY_J = 74,
	KEY_K = 75,
	KEY_L = 76,
	KEY_M = 77,
	KEY_N = 78,
	KEY_O = 79,
	KEY_P = 80,
	KEY_Q = 81,
	KEY_R = 82,
	KEY_S = 83,
	KEY_T = 84,
	KEY_U = 85,
	KEY_V = 86,
	KEY_W = 87,
	KEY_X = 88,
	KEY_Y = 89,
	KEY_Z = 90
};

enum ControllerButtons
{
	LeftTrigger = 0,
	RightTrigger = 1,
	ButtonA = XINPUT_GAMEPAD_A,
	ButtonB = XINPUT_GAMEPAD_B,
	ButtonX = XINPUT_GAMEPAD_X,
	ButtonY = XINPUT_GAMEPAD_Y,
	ButtonStart = XINPUT_GAMEPAD_START,
};

class Controller
{
public:
	XINPUT_CAPABILITIES caps;
	XINPUT_STATE state;
	int id;

	void Update()
	{
		//gets the state
		int res = XInputGetState(id, &state);
		if (res != 0)
		{
			//controller was disconnected
		}
	}

	//right high frequency
	//left low freq
	void SetVibrate(float left, float right)
	{
		//caps.Vibration 
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed = left * 65535.0f;
		vib.wRightMotorSpeed = right * 65535.0f;
		XInputSetState(id, &vib);
	}

	Vec3 GetLeftStick()
	{
		Vec3 ret;
		ret.x = (float)this->state.Gamepad.sThumbLX / (65535.0f / 2.0f);
		ret.y = (float)this->state.Gamepad.sThumbLY / (65535.0f / 2.0f);
		ret.z = 0;
		return ret;
	}

	Vec3 GetRightStick()
	{
		Vec3 ret;
		ret.x = (float)this->state.Gamepad.sThumbRX / (65535.0f / 2.0f);
		ret.y = (float)this->state.Gamepad.sThumbRY / (65535.0f / 2.0f);
		ret.z = 0;
		return ret;
	}
};

#include <functional>

class CInput
{
public:
	POINT m_pos;
#ifdef _WIN32
	HWND window;
#endif

	bool left_mouse;
	bool right_mouse;

	bool lmouse_down;
	bool rmouse_down;

	int deltaX;
	int deltaY;

#ifdef _WIN32
	CInput()
	{
		this->first_player_controller = false;
		bindings[BIND_FIRE] = Binding(BindingType::Mouse, true, RightTrigger);
		bindings[BIND_JUMP] = Binding(BindingType::Keyboard, KEY_SPACE, XINPUT_GAMEPAD_B);
		bindings[BIND_RELOAD] = Binding(BindingType::Keyboard, KEY_R, XINPUT_GAMEPAD_X);
		bindings[BIND_USE] = Binding(BindingType::Keyboard, KEY_E, XINPUT_GAMEPAD_A);
		bindings[BIND_Q] = Binding(BindingType::Keyboard, KEY_Q);
		bindings[BIND_Z] = Binding(BindingType::Keyboard, KEY_Z, LeftTrigger);//zoom
		bindings[BIND_C] = Binding(BindingType::Keyboard, KEY_C);
		bindings[BIND_VIEW] = Binding(BindingType::Keyboard, KEY_T, XINPUT_GAMEPAD_Y);
		bindings[BIND_SHIFT] = Binding(BindingType::Keyboard, KEY_SHIFT);
	};
#else
	CInput()//HWND wind)
	{
		//this->window = wind;
	};
#endif

	void Update()
	{
#ifdef _WIN32
		GetCursorPos(&m_pos);
		ScreenToClient(window, &m_pos);
#endif
		this->UpdateControllers();
	};
	enum class BindingType
	{
		Keyboard,
		Mouse
	};

	//each can be bound to a controller and a mouse/kb
	struct Binding
	{
		BindingType type;
		union
		{
			int key;//for keyboard
			bool left;//for mouse
		};
		int button;//for controller

		bool oldstate[4];

		Binding() {}
		Binding(BindingType type, bool lmb, int button = 0) : type(type), left(lmb), button(button) { for (int i = 0; i < 4; i++) oldstate[i] = false; }
		Binding(BindingType type, int key, int button = 0) : type(type), key(key), button(button) { for (int i = 0; i < 4; i++) oldstate[i] = false; }
	};

	bool first_player_controller;
	std::map<int, Binding> bindings;
	bool GetBind(int player, int bind)
	{
		auto binding = bindings[bind];
		bool state = false;
		if (player == 0)
		{
			switch (binding.type)
			{
			case BindingType::Keyboard:
				state = this->kb[binding.key];// ? 1.0f : 0.0f;
				break;
			case BindingType::Mouse:
				state = binding.left ? this->lmouse_down : this->rmouse_down;
				break;
			}
		}

		if (this->controllers.size() > 0 && state == false)
		{//only check controller if kb or mouse not pressed
			if (this->controllers.size() == 1 && player == 1 && this->first_player_controller)
				return state;

			int cid = first_player_controller ? 0 : player;//controller id
			if (cid >= this->controllers.size())
				return state; 

			switch (binding.button)
			{
			case LeftTrigger:
				return this->controllers[cid].state.Gamepad.bLeftTrigger > 125;
			case RightTrigger:
				return this->controllers[cid].state.Gamepad.bRightTrigger > 125;
			default:
				return this->controllers[cid].state.Gamepad.wButtons & binding.button;
			}
		}

		//return whatever state is true, controller or kb
		return state;
	}

	void DoCallbacks(std::function<void(int, int)> bindpresscb)
	{
		//update state
		for (int i = 0; i < 2; i++)
		{
			for (auto ii : this->bindings)
			{
				bool New = this->GetBindBool(i, ii.first);
				bool Old = ii.second.oldstate[i];

				if (Old == false && New == true)
					bindpresscb(i, ii.first);

				this->bindings[ii.first].oldstate[i] = New;
			}
		}

		if (controller_change)
		{
			//do that callback
			printf("A controller was connected/disconnected.\n");
			this->controller_change = false;
		}
	}

	//and add on press bindings, need a callback system for this

	//0 = move left/right, 1 = move forward/back, 2 = look right/left 3 = look up/down
	float GetAxis(int player, int axis)
	{
		bool invertlooky = true;
		//axes will be hardcoded for now
		bool controller = this->controllers.size() > 0;
		int controllerid = this->first_player_controller ? player : player - 1;
		if (controllerid >= this->controllers.size() || controllerid < 0)
			controller = false;
		float res = 0;
		switch (axis)
		{
		case 0:
			if (player == 0 && controller == false)
			{
				if (kb[KEY_D])
					res = 1;
				if (kb[KEY_A])
					res -= 1;
			}
			else
			{
				if (controller)
				{
					Vec3 dir = this->controllers[controllerid].GetLeftStick();
					if (abs(dir.x) > 0.2f)
						res = dir.x;
				}
			}
			break;
		case 1:
			if (player == 0 && controller == false)
			{
				if (kb[KEY_W])
					res = 1;
				if (kb[KEY_S])
					res -= 1;
			}
			else
			{
				if (controller)
				{
					Vec3 dir = this->controllers[controllerid].GetLeftStick();
					if (abs(dir.y) > 0.2f)
						res = dir.y;
				}
			}
			break;
		case 2:
			if (player == 0 && controller == false)
			{
				//use mouse for now
				return this->deltaX;
			}
			else
			{
				if (controller)
				{
					Vec3 dir = this->controllers[controllerid].GetRightStick();
					if (dir.x > 0.15f)
						res = ((dir.x - 0.15f)*1.0f / 0.85f)*6.5f;//hack!
					else if (dir.x < -0.15f)
						res = ((dir.x + 0.15f)*1.0f / 0.85f)*6.5f;
				}
			}
			break;
		case 3:
			if (player == 0 && controller == false)
			{
				//use mouse for now
				return this->deltaY;
			}
			else
			{
				if (controller)
				{
					Vec3 dir = this->controllers[controllerid].GetRightStick();
					dir.y = invertlooky ? -dir.y : dir.y;
					if (dir.y > 0.15f)
						res = ((dir.y - 0.15f)*1.0f / 0.85f)*6.5f;//hack!
					else if (dir.y < -0.15f)
						res = ((dir.y + 0.15f)*1.0f / 0.85f)*6.5f;
				}
			}
			break;
		}
		return res;
	}

	bool GetBindBool(int player, int bind)
	{
		return GetBind(player, bind) > 0.5f ? true : false;
	}

	bool controller_change;
	std::vector<Controller> controllers;
	void UpdateControllers()
	{
		int oldsize = this->controllers.size();
		controllers.clear();
		for (int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));
			auto res = XInputGetState(i, &state);
			if (res == ERROR_SUCCESS)
			{
				//connected
				Controller c;
				c.state = state;
				c.id = i;
				XInputGetCapabilities(i, 0, &c.caps);
				controllers.push_back(c);
			}
			else
			{
				//not connected

			}
		}
		if (oldsize != controllers.size())
			controller_change = true;
	}

	bool* kb;
};