#ifndef MESSAGEBOX_HEADER
#define MESSAGEBOX_HEADER

#include "gui_window.h"
#include <JetEngine/Graphics/CRenderer.h>
#include "gui_button.h"

extern CRenderer* renderer;

class gui_messagebox: public gui_window
{
	gui_button button;
public:
	gui_messagebox() {
		this->button.setpos(0,0);
		this->button.setsize(100,40);
		this->button.settext("Ok");
		this->button.callback = [](){ log("hit ok button\n");};
		this->AddWindow(&this->button);
	};
	~gui_messagebox() {};

	char text[256];

	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	};

	void setcolor(COLOR color)
	{
		this->m_inactivecolor = color;
	};

	void settext(char* text)
	{
		strncpy(this->text, text,256);
	};

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		int size = abs(this->m_position.left - this->m_position.right);
		int vsize = abs(this->m_position.top - this->m_position.bottom);
		this->button.setpos(size/2 - 50, vsize - 60);
		renderer->SetPixelTexture(0,(Texture)0);
		renderer->DrawRect(&this->m_position,COLOR_ARGB(255,128,128,128));
		Rect re = this->m_position;
		re.bottom = re.top + 30;
		renderer->DrawRect(&re, COLOR_ARGB(255,64,64,64));

		//draw title
		Rect r = this->m_position;
		r.bottom = r.top + 40;
		renderer->DrawCenteredText(r,this->m_caption, this->m_inactivecolor);

		Rect r2 = this->m_position;
		r2.top = r.top + 80;
		r2.bottom = r.top + 40;
		renderer->DrawCenteredText(r2,this->text, this->m_inactivecolor);
		return 1;
	}; 

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

			this->getparent()->RemoveWindow(this);
		}
		return 1;
	}; 
	virtual int  wm_keydown(int key) 
	{
		return 1;
	}; 
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif