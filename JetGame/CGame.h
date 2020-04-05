#ifndef GAME_HEADER
#define GAME_HEADER

#include "../JetEngine/gui/gui_window.h"
#include "../JetEngine/CInput.h"
#include "../JetEngine/Util/CTimer.h"
#include "../JetEngine/Sound/SoundManager.h"
#include "../JetEngine/Defines.h"

#include <vector>
#include <map>
#include <string.h>

#undef MessageBox

class Window;

struct Setting
{
	std::string name;
	std::string string;
	bool b;
	float i;
};

class CGameState;

extern CRenderer* renderer;

#include "../JetEngine/camera.h"

//your game should derive this and override the virtuals
class CGame
{
public:
	void Init(Window* window);
	virtual void onInit() = 0;

	void Cleanup();

	void ChangeState(CGameState* state);
	void PushState(CGameState* state);
	void PopState();

	void MouseEvent(int x, int y, int eventId);
	void KeyboardEvent(int eventType, int keyId);
	void TouchEvent(int eventType, float x, float y, float dx, float dy);

	void Pause();
	void Resume();

	void HandleEvents(int messagetype, void* data1, void* data2);
	void Update();
	virtual void onUpdate() = 0;

	//connected players/controllers
#undef min
	int GetPlayerCount()
	{
		int controllers = input_.controllers.size();
		int num_players = (input_.first_player_controller ? controllers : controllers + 1);
		if (num_players == 0)
			num_players = 1;
		return num_players > 4 ? 4 : num_players;
	}

	//gets the controller id for a given player number
	int GetPlayerControllerId(int player)
	{
		return input_.first_player_controller ? player + 1 : player;
	}

private:
	void Draw();

public:
	bool Running() { return running_; }
	void Quit() { running_ = false; }

	std::map<int, Setting> settings;
	void LoadSettings()
	{
#ifndef ANDROID
		FILE* f = fopen("settings.cfg", "rb");
		if (f)
		{
			printf("found config file\n");
			fseek(f,0,SEEK_END);   // non-portable
			int size = ftell(f);
			fseek(f,0,SEEK_SET);
			char* d = new char[size+1];
			fread(d, size, 1, f);
			d[size-1] = 0;
			
			char* pch = strtok(d,"\n");
			while (pch != NULL)
			{
				logf("read setting %s\n",pch);

				char name[50]; char val[50];
				float value = -555;
				sscanf(pch, "%s %s", name, val);
				sscanf(pch, "%s %f", name, &value);

				int k = 0;
				int l = strlen(name);
				name[(l--)-1] = 0;
				for (int i = 0; i < l; i++)
				{
					k += name[i];
					k *= -76;
				}

				if (l != 0)
				{
					Setting s;
					s.b = false;
					s.i = value;
					s.name = name;
					if (value == -555)
						s.string = val;

					this->settings[k] = s;
				}

				pch = strtok(NULL, "\n");
			}

			fclose(f);
			delete[] d;
		}
#endif
	}

	void SaveSettings()
	{
#ifndef ANDROID
		printf("Saving CVars...\n");
		FILE* f = fopen("settings.cfg", "w+");
		int ptr = 0;
		char str[2000];
		for (auto ii = this->settings.begin(); ii != this->settings.end(); ii++)
		{
			if (ii->second.string.length() > 0)
				ptr += sprintf(str+ptr, "%s: %s\n", ii->second.name.c_str(), ii->second.string.c_str());
			else
				ptr += sprintf(str+ptr, "%s: %f\n", ii->second.name.c_str(), ii->second.i);
		}
		fwrite(str, strlen(str), 1, f);
		fclose(f);
#endif
	}

	void RegisterCVar(const char* name, float default)
	{
		int k = 0;
		int l = strlen(name);
		for (int i = 0; i < l; i++)
		{
			k += name[i];
			k *= -76;
		}
		this->settings[k].i = default;
		this->settings[k].name = name;
		logf("Registered CVar: %s with default: %f\n", name, default);
	}

	void RegisterCVar(const std::string name, const std::string default)
	{
		int k = 0;
		int l = name.length();
		for (int i = 0; i < l; i++)
		{
			k += name[i];
			k *= -76;
		}
		this->settings[k].i = 0;
		this->settings[k].name = name;
		this->settings[k].string = default;
		logf("Registered CVar: %s with default: %s\n", name.c_str(), default.c_str());
	}

	float GetSettingFloat(const char* name)
	{
		int k = 0;
		int l = strlen(name);
		for (int i = 0; i < l; i++)
		{
			k += name[i];
			k *= -76;
		}

		auto ii = settings.find(k);
		if (ii != settings.end())
			return ii->second.i;
		//need to insert if not present
		throw 7;
		return 0;
	}

	std::string GetSettingString(const char* name)
	{
		int k = 0;
		int l = strlen(name);
		for (int i = 0; i < l; i++)
		{
			k += name[i];
			k *= -76;
		}

		auto ii = settings.find(k);
		if (ii != settings.end())
			return ii->second.string;
		//need to insert if not present
		throw 7;
		return 0;
	}

	bool GetSettingBool(const char* name)
	{
		int k = 0;
		int l = strlen(name);
		for (int i = 0; i < l; i++)
		{
			k += name[i];
			k *= -76;
		}

		auto ii = settings.find(k);
		if (ii != settings.end())
			return ii->second.i != 0.0f;
		//need to insert if not present
		throw 7;
		return false;
	}

	bool ProcessCommand(const char* command)
	{
		char name[50]; char val[50];
		float value;
		sscanf(command, "%s %s", name, val);

		int k = 0;
		int l = strlen(name);
		for (int i = 0; i < l; i++)
		{
			k += name[i];
			k *= -76;
		}

		auto ii = settings.find(k);
		if (ii != settings.end())
		{
			if (val[0] != '-' && !(val[0] >= '0' && val[0] <= '9'))
			{
				ii->second.string = val;
			}
			else
			{
				sscanf(command, "%s %f", name, &value);
				ii->second.i = value;
			}
			return true;
		}

		return false;
	}

	void Dialog(gui_window* window)
	{
		base_gui_.AddWindow(window);
	}
	void MessageBox(char* caption, char* text);

	CInput* GetInput();

	Window* GetWindow()
	{
		return window_;
	}

	bool keyboard[256];

	void SetView(const CCamera* cam)
	{
		add_camera_ = *cam;
	}

	void SetClearColor(float a, float r, float g, float b)
	{
		add_clear_color_ = { a, r, g, b };
	}

	void Resize(int x, int y)
	{
		// tell the render thread to perform a resize
		xres_ = x;
		yres_ = y;
		needs_resize_ = true;
	}

	void RenderLoop();

private:
	std::thread render_thread_;

	// the stack of states
	std::vector<CGameState*> states_;
	std::vector<CGameState*> to_delete_;

	//used for a latch to make sure theres an update before a render
	CGameState* last_;

	//the input manager
	CInput input_;

	//overarching gui_manager for messageboxes
	gui_window base_gui_;

	//timer for calculating dt and framerate
	CTimer timer_;

	//the time since the last frame started, used for dt
	float elapsedtime_;

	//the window that this game is running in
	Window* window_;

	// Threaded render stuff (some of this should probably move)
	// the add variables are what should be set for the next frame
	// the processs variables are used by the thread

	CCamera add_camera_, process_camera_;
	Vec4 add_clear_color_;
	Vec4 process_clear_color_;
	bool needs_resize_ = false;
	int xres_, yres_;

	bool running_;
	bool fullscreen_;
};

#endif