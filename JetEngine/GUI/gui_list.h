#ifndef GaUILISTVIEW_HEADER
#define GaUILISTVIEW_HEADER

#include "gui_window.h"
#include "gui_button.h"
#include "gui_vertical_scrollbar.h"
#include "../Graphics/CRenderer.h"

extern CRenderer* renderer;

class gui_list : public gui_window
{
	int paddingH;
	int paddingV;

	gui_vertical_scrollbar scroll;

public:

	gui_list()
	{
		this->paddingV = 5;
		this->paddingH = 5;

		this->AddWindow(&this->scroll);
	};
	~gui_list() { this->RemoveWindow(&this->scroll); };

	int current_y = 0;
	void AddItem(gui_window* window)
	{
		window->setpos(0, current_y);
		current_y += window->getheight() + paddingV;
		this->AddWindow(window);

		//reset scroll bar
		this->scroll.SetRange(current_y);
	}

	void setPadding(int pad)
	{
		this->paddingH = pad;
	}

	void doLayout()
	{
		int len = current_y - this->getheight();
		if (len < 0)
			len = 0;
		this->scroll.SetRange(len);

		int lastpos = this->paddingV;
		for (auto it = ++m_subwins.begin(); it != m_subwins.end(); it++)
		{
			gui_window* win = 0;
			win = *it;
			if (win == 0)
				continue;
			win->setpos(this->paddingH, lastpos - scroll.position);
			if (lastpos - scroll.position + win->getheight() < 0 || lastpos - scroll.position > this->getheight())
				win->Hide();
			else
				win->Show();
			lastpos += paddingV + win->getheight();	
		}

		this->scroll.setsize(30, this->getheight());
		this->scroll.setpos(this->getwidth() - 30, 0);
	}

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		this->doLayout();

		Rect r = this->m_position;
		r.left += x;
		r.right += x;
		r.top += y;
		r.bottom += y;
			
		renderer->SetPixelTexture(0, (Texture)0);
		renderer->DrawRect(&r, 0x66444444);

		return 1;
	};

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 

	//virtual int  wm_keydown(int key); 
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif