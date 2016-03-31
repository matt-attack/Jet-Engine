
#ifndef GUIBUTTON_HEADER
#define GUIBUTTON_HEADER

#include "gui_window.h"
#include "../Sound/SoundObject.h"
#include "../Sound/SoundManager.h"
#include "../Graphics/CRenderer.h"
#include <functional>

extern CRenderer* renderer;

class gui_button: public gui_window
{
public:
	gui_button()
	{
		this->text = 0;
		//this->callback = 0;
	}

	~gui_button()
	{
		if (this->text)
			delete[] this->text;
	}

	char* text;

	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	}

	void settext(char* txt)
	{
		if (this->text)
			delete[] this->text;

		unsigned int l = strlen(txt);
		this->text = new char[l+1];
		strcpy(this->text, txt);
	}

	//virtual void callback() = 0;
	//void (*callback)();
	std::function<void()> callback;

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		Rect r = this->m_position;
		r.top += y;
		r.bottom += y;
		r.left += x;
		r.right += x;

		renderer->SetPixelTexture(0, renderer->gui_texture);

		unsigned long color = 0xFFFFFFFF;
		if (mouseover)
		{
			color = 0xFFFF00FF;
		}
		//renderer->SetCullmode(CULL_NONE);
		renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, color);
		renderer->DrawCenteredText(r, this->text, 0xFFFFFFFF);//FFFFFFFF);
		//renderer->DrawText(this->m_position.left + 10, this->m_position.top, this->text, 0xFFFFFFFF);
		return 1;
	}

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y);
	//virtual int  wm_ldrag(coord x, coord y);
	virtual int wm_lclick(coord x, coord y)
	{
		log("clicked button\n");
		SoundManager::GetInstance()->GetSound("select")->Play();
		SoundManager::GetInstance()->GetSound("select")->SetRelative(true);
		
		if (callback)
		{
			this->callback();
		}
		return 1;
	}

	virtual int  wm_keydown(int key)
	{
		return 1;
	}
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); };
	//virtual int  wm_cansize(coord x, coord y);
	//virtual int  wm_size(coord x, coord y, int cansize);
	//virtual int  wm_sizechanged(void) { return(0); }
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif