#include "CInput.h"

#include "Defines.h"
//#include "Math/Vector.h"


CInput::CInput()
{
	this->first_player_controller = false;
}

void CInput::Update()
{
#ifdef _WIN32
	GetCursorPos(&m_pos);
	ScreenToClient(window, &m_pos);
#endif
	this->UpdateControllers();
}

void CInput::UpdateControllers()
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


//this is where it all happens, gets the status for each binding
float CInput::GetBindingState(int player, const Binding& binding)
{
	float state = 0;
	if (player == 0)
	{
		switch (binding.type)
		{
		case BindingType::Keyboard:
			state = this->kb[binding.key] ? 1.0f : 0.0f;
			break;
		case BindingType::Mouse:
			state = (binding.left ? this->lmouse_down : this->rmouse_down) ? 1.0f : 0.0f;
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
			return ((float)this->controllers[cid].state.Gamepad.bLeftTrigger) / 255.0f;// > 125;
		case RightTrigger:
			return ((float)this->controllers[cid].state.Gamepad.bRightTrigger) / 255.0f;// > 125;
		default:
			return (this->controllers[cid].state.Gamepad.wButtons & binding.button) ? 1.0f : 0.0f;
		}
	}

	//return whatever state is true, controller or kb
	return state;
}

void CInput::BindAxis(int id, const Binding up, const Binding down)
{
	AxisBinding b;
	b.axis = 0;
	b.up = up;
	b.down = down;
	this->axes[id] = b;
}

float CInput::GetAxis(int player, int axis)
{
	bool invertlooky = true;
	//axes will be hardcoded for now
	bool controller = this->controllers.size() > 0;
	int controllerid = this->first_player_controller ? player : player - 1;
	if (controllerid >= this->controllers.size() || controllerid < 0)
		controller = false;

	auto& axis_data = this->axes[axis];
	float bs = 0.0f;
	if (axis_data.axis)
	{

	}
	else
	{
		bs += this->GetBindingState(player, axis_data.up);
		bs -= this->GetBindingState(player, axis_data.down);
	}

	float res = 0;
	switch (axis)
	{
	case 0:
		if (player == 0 && controller == false)
		{
			res = bs;
			//if (kb[KEY_D])
			//	res = 1;
			//if (kb[KEY_A])
			//	res -= 1;
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
			res = bs;
			//if (kb[KEY_W])
			//	res = 1;
			//if (kb[KEY_S])
				//res -= 1;
		}
		else
		{
			if (controller)
			{
				Vec3 dir = (this->controllers[controllerid].GetLeftStick());
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

bool CInput::GetBindBool(int player, int bind)
{
	auto binding = bindings[bind];

	return this->GetBindingState(player, binding) >= 0.5f;

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

void CInput::DoCallbacks(std::function<void(int, int)> bindpresscb)
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

void CInput::Bind(int id, const Binding b)
{
	this->bindings[id] = b;
}
