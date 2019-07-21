#include "CGame.h"
#include <JetEngine/Graphics/CRenderer.h>
#include <JetEngine/gui/gui_messagebox.h>
#include "gui/gui_settings.h"
#include <JetEngine/Sound/SoundObject.h>
#include <JetEngine/Graphics/Renderer.h>
#include <JetEngine/ResourceManager.h>
#include <JetEngine/Util/Profile.h>

#include "CGameState.h"
#include "Window.h"

#ifdef _WIN32
#include <Psapi.h>
#endif

#include <AL\al.h>

#define USE_RENDER_THREAD

void initKeyboard(HWND han_Window)//should be setup input
{
	RAWINPUTDEVICE Rid[2];
	Rid[1].usUsagePage = (USHORT)0x01;
	Rid[1].usUsage = (USHORT)0x02;
	Rid[1].dwFlags = 0;//RIDEV_NOLEGACY;//RIDEV_INPUTSINK | RIDEV_CAPTUREMOUSE;
	Rid[1].hwndTarget = han_Window;
	Rid[0].usUsagePage = (USHORT)0x01;
	Rid[0].usUsage = (USHORT)0x04;
	Rid[0].dwFlags = 0;//RIDEV_NOLEGACY;//RIDEV_INPUTSINK | RIDEV_CAPTUREMOUSE;
	Rid[0].hwndTarget = han_Window;
	bool res = RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));
	if (!res)
		log("[ERROR] Raw input initiation failed\n");
}

void OSInit(HWND hWnd)
{
	initKeyboard(hWnd);

	XInputEnable(true);
}

#include <JetEngine/Graphics/VRRenderer.h>
#include <JetEngine/IMaterial.h>

#include <JetEngine/Graphics/RenderTexture.h>

void CGame::RenderLoop()
{
#ifdef USE_RENDER_THREAD
	while (true)
#endif
	{
		// wait until we get data to begin
#ifdef USE_RENDER_THREAD
		r.start_lock_.wait();
#endif
		if (this->needs_resize_)
		{
			//if (renderer)
			renderer->Resize(this->xres_, this->yres_);
			this->needs_resize_ = false;
		}

		// swap buffers
		std::swap(this->add_camera_, this->process_camera_);
		std::swap(this->add_clear_color_, this->process_clear_color_);

		renderer->shader = 0;//makes shader reloading work

		VRRenderer* vr = dynamic_cast<VRRenderer*>(renderer);

		//update render settings
		r.EnableShadows(this->GetSettingBool("cl_shadows"));// todo make sure to not sure this outside of the renderer
		float samples = this->GetSettingFloat("cl_aa_samples");
		renderer->SetAALevel(vr ? 0 : samples);
		renderer->EnableVsync(vr ? 0 : this->GetSettingBool("cl_vsync"));
		r.SetMaxShadowDist(this->GetSettingFloat("cl_shadow_dist"));

		//ok, lets be dumb and update materials here
		// todo maybe move this, definately need to move it for VR
		for (auto ii : IMaterial::GetList())
			ii.second->Update(renderer);

		r.ThreadedRender(renderer, &this->process_camera_, this->process_clear_color_);

		renderer->FlushDebug();

		//draw stuff like message boxes
		this->base_gui.renderall(0, 0, input.m_pos.x, input.m_pos.y, 0);

		if (showdebug)
		{
			renderer->SetFont("Arial", 20);

			unsigned int mem = 0;
#ifdef _WIN32
			PROCESS_MEMORY_COUNTERS pmc;
			GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
			mem = pmc.WorkingSetSize;
#endif

			float rft = 0;
			if (Profiles.find({ "FrameTime", 0 }) != Profiles.end())
				rft = Profiles[{"FrameTime", 0}]->average;
			float rp = 0;
			if (Profiles.find({ "RendererProcess", 0 }) != Profiles.end())
				rp = Profiles[{"RendererProcess", 0}]->average;
			renderer->DrawStats(1 / this->timer.GetFPS(), rft, mem, rp);

			if (showdebug >= 2)
				ProfilesDraw();
		}

		renderer->ResetStats();

		{
			PROFILE("Present");
			GPUPROFILE("Present");
			renderer->Present();//exclude this from rft
		}
	}
}

void CGame::Init(Window* window)
{
	//need escape and back button also to only do rising edge of controller buttons (A) for menus
	this->input.kb = this->keyboard;

	OSInit((HWND)window->GetOSHandle());

	this->window = window;

	this->input.kb = this->keyboard;
	for (int i = 0; i < 256; i++)
		this->keyboard[i] = false;

	//	maybe make these static? so they can be placed anywhere
	log("Registering CVars\n");
	this->RegisterCVar("cl_shadows", 1);
	this->RegisterCVar("cl_vsync", 0);
	this->RegisterCVar("cl_name", "Player");
	this->RegisterCVar("cl_shadow_dist", 150);
	this->RegisterCVar("cl_volume", 50);
	this->RegisterCVar("cl_controller", 1);
	this->RegisterCVar("cl_aa_samples", 1);

	//need to start making map with mission like thing
	//improve mech config

	log("Initializing Sound Manager\n");
	SoundManager::GetInstance()->Initialize("hello", false);
	SoundManager::GetInstance()->Enable();
	SoundManager::GetInstance()->SetMasterVolume(0.5f);
	
	SoundManager::GetInstance()->Update();

	resources.init();

	r.Init(renderer);//initialize the pipeline

	// startup the renderer thread, if we want multithreaded rendering
#ifdef USE_RENDER_THREAD
	render_thread_ = std::thread([this]() {
		this->RenderLoop();
	});
#endif

	this->timer.Start();

#ifdef _WIN32
	this->input.window = (HWND)window->GetOSHandle();
#endif

	this->onInit();

	this->LoadSettings();//load after on init so new cvars load properly

	this->m_running = true;

	showdebug = 0;
	this->last = 0;
}

void CGame::Cleanup()
{
	this->SaveSettings();

	// cleanup the all states
	while (!states.empty()) {
		states.back()->Cleanup();
		delete states.back();
		states.pop_back();
	}

	SoundManager::Destroy();

	ProfileExit();

	resources.release_unused();//remove resources

	printf("CGameEngine Cleanup\n");
}

void CGame::ChangeState(CGameState* state)
{
	// cleanup the current state
	if (!states.empty()) {
		states.back()->Cleanup();
		//queue it to delete on next update
		to_delete.push_back(states.back());

		states.pop_back();
	}

	// store and init the new state
	states.push_back(state);
	states.back()->Init(this);
}

void CGame::PushState(CGameState* state)
{
	// pause current state
	if (!states.empty()) {
		states.back()->Pause();
	}

	// store and init the new state
	states.push_back(state);
	states.back()->Init(this);
}

void CGame::PopState()
{
	// cleanup the current state
	if (!states.empty()) {
		states.back()->Cleanup();
		//then delete it!
		this->to_delete.push_back(states.back());

		//delete states.back();
		states.pop_back();
	}

	// resume previous state
	if (!states.empty()) {
		states.back()->Resume();
	}
}

void CGame::HandleEvents(int messagetype, void* data1, void* data2)
{
	// let the state handle events
	states.back()->HandleEvents(this, messagetype, data1, data2);
}

void CGame::MouseEvent(int x, int y, int eventId)
{
	if (eventId == ENG_R_DOWN)
	{
		this->input.rmouse_down = true;
	}
	else if (eventId == ENG_L_DOWN)
	{
		this->input.lmouse_down = true;

		if (this->base_gui.wm_lbuttondown(x, y))
			return;//dont bother passing it down if we capture it
	}
	else if (eventId == ENG_R_UP)
	{
		this->input.rmouse_down = false;

		if (this->base_gui.wm_rclick(x, y))
			return;//dont bother passing it down if we capture it
	}
	else if (eventId == ENG_L_UP)
	{
		this->input.lmouse_down = false;

		if (this->base_gui.wm_lclick(x, y))
			return;//dont bother passing it down, we captured it
	}
	else if (eventId == ENG_L_DRAG)
	{
		if (this->base_gui.wm_ldrag(x, y))
			return;//dont bother passing it down if we capture it
	}

	states.back()->MouseEvent(this, x, y, eventId);
}

void CGame::TouchEvent(int eventType, float x, float y, float dx, float dy)
{
	states.back()->TouchEvent(this, eventType, x, y, dx, dy);
}

void CGame::KeyboardEvent(int state, int key)
{
	if (state == ENG_KEY_DOWN)
	{
		this->keyboard[key] = true;

		this->base_gui.wm_keydown(key);

#ifndef ANDROID
		if (key == VK_F3)
			showdebug = (showdebug + 1) % 5;
#endif
	}
	else if (state == ENG_KEY_UP)
	{
		this->keyboard[key] = false;
	}
	else if (state == ENG_CHAR)
	{
		this->base_gui.wm_char(key);
	}

	states.back()->KeyboardEvent(this, state, key);
}

void CGame::Pause()
{
	if (states.size() > 0)
		states.back()->Pause();
}

void CGame::Resume()
{
	if (states.size() > 0)
		states.back()->Resume();
}

void CGame::Update()
{
	ProfileStartFrame();
	RPROFILE("FrameTime");
	GPUPROFILE("FrameTime");

	//delete old states
	for (size_t i = 0; i < this->to_delete.size(); i++)
		delete this->to_delete[i];
	this->to_delete.clear();

	this->window->ProcessMessages();

	//update input stuff
	this->input.Update();

	if (states.size() > 0)
	{
		auto state = states.back();
		this->input.DoCallbacks([this, state](int player, int bind) { state->BindPress(this, player, bind);  });
		this->input.first_player_controller = this->GetSettingBool("cl_controller");
	}

	//update settings
	SoundManager::GetInstance()->SetMasterVolume(this->GetSettingFloat("cl_volume") / 100.0f);

	SoundManager::GetInstance()->Update();

	//was using LINEAR_DISTANCE_CLAMPED
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);// _CLAMPED);

	this->timer.Update();

	elapsedtime = timer.GetElapsedTime();

	resources.update();//this polls for changes in the filesystem

	//should I call this here, or later???
	this->onUpdate();

	// let the state update the game
	if (states.size() > 0)
		states.back()->Update(this, elapsedtime);

	if (states.size() > 0 && this->states.back() != this->last)//prevents rendering right after gamestate changed because update hasnt yet ran
	{
		this->last = this->states.back();
		return;
	}

	this->Draw();//render the frame

	//reset input
	input.EOFUpdate();
	input.deltaX = 0;
	input.deltaY = 0;
	input.left_mouse = false;
	input.right_mouse = false;

	//this is a dumb not crossplatform part
#ifdef WIN32
	auto res = GetFocus();
	if (states.size() > 0 && res != this->window->GetOSHandle())
		this->states.back()->Pause();
#endif
}


void CGame::Draw()
{
	// let the state draw the screen
	{
		GPUPROFILEGROUP("Game Render");
		if (states.size() > 0)
			states.back()->Draw(this, elapsedtime);
	}
#ifndef USE_RENDER_THREAD
	this->RenderLoop();
#else
	// wait for the renderer to be free so we dont get ahead of ourselves
	r.start_lock_.notify();

	// theres a bit of free time here if you want to do something...

	r.renderable_lock_.wait();// wait for the renderer to start before moving to the next frame
#endif
}

CInput* CGame::GetInput()
{
	return &this->input;
}

void CGame::MessageBox(char* caption, char* text)
{
	gui_messagebox* m = new gui_messagebox;
	int minsize = renderer->TextSize(text) + 20;
	if (minsize < 200)
		minsize = 200;

	m->setpos(renderer->xres / 2.0f - minsize / 2, renderer->yres / 2.0f - minsize / 2);
	m->setsize(minsize, 200);
	m->settext(text);
	m->setcaption(caption);
	this->base_gui.AddWindow(m);
}