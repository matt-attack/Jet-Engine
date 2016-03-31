#ifndef SETTINGSBOX_HEADER
#define SETTINGSBOX_HEADER

#include <JetEngine/GUI/gui_window.h>
#include <JetEngine/Graphics/CRenderer.h>
#include "JetEngine/GUI/gui_button.h"
#include "JetEngine/GUI/gui_dropbox.h"
#include <JetEngine/GUI/gui_tabpages.h>
#include <JetEngine/GUI/gui_textbox.h>
#include <JetEngine/GUI/gui_slider.h>
#include <JetEngine/GUI/gui_label.h>
#include "../CGame.h"

class gui_settings: public gui_window
{
	gui_button button;
	gui_tabbox tabs;
	gui_slider volume;
	gui_slider shadowdist;

	CGame* game;
public:

	gui_settings(CGame* game);

	~gui_settings() {}


	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	}

	void setcolor(COLOR color)
	{
		this->m_inactivecolor = color;
	}

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		int size = abs(this->m_position.left - this->m_position.right);
		int vsize = abs(this->m_position.top - this->m_position.bottom);
		this->button.setpos(size/2 - 50, vsize - 50);
		renderer->SetPixelTexture(0, 0);
		renderer->DrawRect(&this->m_position,COLOR_ARGB(255,128,128,128));
		Rect re = this->m_position;
		re.bottom = re.top + 30;
		renderer->DrawRect(&re, COLOR_ARGB(255,64,64,64));

		//draw title
		Rect r = this->m_position;
		r.bottom = r.top + 30;
		renderer->DrawCenteredText(r,"Options", this->m_inactivecolor);

		Rect r2 = this->m_position;
		r2.top = r.top + 80;
		r2.bottom = r.top + 40;
		//renderer->DrawCenteredText(r2,this->text, this->m_inactivecolor);
		return 1;
	}

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 
	virtual int  wm_lclick(coord x, coord y) 
	{
		this->screentoclient(x,y);

		gui_window* w = this->findchildatcoord(x,y,0);
		if (w)
		{
			w->wm_lclick(x,y);

			if (w == &this->button)
			{
				this->getparent()->RemoveWindow(this);
				delete this;
			}
		}
		return 1;
	}

	/*virtual int  wm_keydown(int key) 
	{
	return 1;
	}*/
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif