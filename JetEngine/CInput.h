#pragma once

#ifndef _WIN32
struct POINT
{
	int x,y;
};
#else
#endif


#include <Windows.h>
#include <XInput.h>
#include <vector>
#include <map>
#include <functional>

#include "Math/Vector.h"

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

	KEY_1 = 0x30,
	KEY_2 = 0x31,
	KEY_3 = 0x32,
	KEY_4 = 0x33,
	KEY_5 = 0x34,

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
	ButtonLeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
	ButtonRightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	ButtonA = XINPUT_GAMEPAD_A,
	ButtonB = XINPUT_GAMEPAD_B,
	ButtonX = XINPUT_GAMEPAD_X,
	ButtonY = XINPUT_GAMEPAD_Y,

	ButtonBack = XINPUT_GAMEPAD_BACK,
	ButtonStart = XINPUT_GAMEPAD_START,
	ButtonLeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,
	ButtonRightThumb = XINPUT_GAMEPAD_RIGHT_THUMB,
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

//each can be bound to a controller and a mouse/kb
enum class BindingType
{
	Keyboard,
	Mouse
};
struct Binding
{
	BindingType type;
	union
	{
		int key;//for keyboard
		bool left;//for mouse
	};
	int button;//for controller
	int joy_button;//for joystick or non XInput controller, starts at 1
	std::string vr_action;

	bool oldstate[4];//one for each player

	Binding() {}
	Binding(BindingType type, bool lmb, int button = 0, int joy_button = -1, std::string vr_action = "") : type(type), left(lmb), button(button), joy_button(joy_button), vr_action(vr_action) { for (int i = 0; i < 4; i++) oldstate[i] = false; }
	Binding(BindingType type, int key, int button = 0, int joy_button = -1, std::string vr_action = "") : type(type), key(key), button(button), joy_button(joy_button), vr_action(vr_action) { for (int i = 0; i < 4; i++) oldstate[i] = false; }
};

struct RawDevice
{
	bool got_data = false;
	int num_buttons = 0;
	bool buttons[30];

	int num_axes = 0;
	float axes[10];
};

class CInput
{
	friend class Window;
	bool controller_change;
	
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

	CInput();

	void Update();

	void EOFUpdate();

	bool first_player_controller;

	const std::map<int, Binding>& GetBindings()
	{
		return this->bindings;
	}

private:
	std::map<int, Binding> bindings;


	struct AxisBinding
	{
		int axis = 0;
		Binding up;
		Binding down;
	};
	std::map<int, AxisBinding> axes;

	float GetBindingState(int player, const Binding& b);
public:

	void Bind(int id, const Binding b);

	void BindAxis(int id, const Binding up, const Binding down);
	enum ControllerAxes
	{
		LeftStickX = 1,
		LeftStickY,
		RightStick,
		LeftTrigger,
		RightTrigger,

		JoystickX,
		JoystickY,
		JoystickZ,
		JoystickThrottle,
	};
	void BindAxis(int id, ControllerAxes axis);

	//ok, todo binds can have floats, so lets change this over
	bool GetBindBool(int player, int bind);

	void DoCallbacks(std::function<void(int, int)> bindpresscb);

	//and add on press bindings, need a callback system for this

	//0 = move left/right, 1 = move forward/back, 2 = look right/left 3 = look up/down
	//others are for joysticks
	float GetAxis(int player, int axis);

	int last_direction[4][4];//0 = center, 1 = up, -1 = down per axis
	int GetAxisDirection(int player, int axis);

	bool UsingJoystick(int player)
	{
		if (player == 0 && active_joystick)
			return true;
		return false;
	}

	//bool GetBindBool(int player, int bind);

	HANDLE active_joystick = 0;
	std::map<HANDLE, RawDevice> devices;
	std::vector<Controller> controllers;
private:
	void UpdateControllers();

public:
	bool* kb;
};