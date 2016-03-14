#ifndef GUILISTVIEW_HEADER
#define GUILISTVIEW_HEADER

#include "gui_window.h"
#include "gui_button.h"
#include "Graphics/CRenderer.h"

extern CRenderer* renderer;

class gui_listview: public gui_window
{
	int paddingH;
	int paddingV;

	gui_button left;
	gui_button right;

public:

	gui_window* selected;
	int scroll;

	gui_listview() 
	{
		this->paddingV = 30;
		this->paddingH = 30;

		this->scroll = 0;
		this->selected = 0;

		right.setsize(20,15);
		//right.callback =
		//this->addwindow(&right);

		left.setsize(20,15);
		//this->addwindow(&left);
	};
	~gui_listview() {};

	void setPadding(int pad)
	{
		this->paddingH = pad;
	}

	void internalButtonPress(gui_listview* me, int action)
	{
		if (action == 1)
			this->scroll += 1;
		else
			this->scroll -= 1;
	}

	void doLayout()
	{
		int p = 0;
		int lastpos = scroll;
		for (auto it = m_subwins.begin() ; it != m_subwins.end(); it++ )
		{
			gui_window* win = 0;
			win = *it;//._Ptr->_Myval;
			if (win == 0)
				continue;
			win->setpos( lastpos + p*paddingH, paddingV);
			lastpos += p*paddingH + (win->m_position.right - win->m_position.left);
			p += 1;
		}
	}

	virtual int wm_paint(coord x, coord y, bool mouseover)
	{
		this->doLayout();

		Rect r = this->m_position;
		r.left += x;
		r.right += x;
		r.top += y;
		r.bottom += y;

		renderer->SetPixelTexture(0, (Texture)0);
		renderer->DrawRect(&r, 0x66666666);

		return 1;
	}; 

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 
	virtual int  wm_lclick(coord x, coord y) 
	{
		this->screentoclient(x,y);
		gui_window* win = this->findchildatcoord(x,y,0);
		if (win)
		{
			win->wm_lclick(x,y);

			//ok, lets scroll over to center this
			int midpoint = (this->m_position.right - this->m_position.left)/2;//relative center of me

			int windowpos = win->m_position.left + (win->m_position.right - win->m_position.left)/2;//center of window
			this->scroll += midpoint - windowpos;

			this->selected = win;
		}

		return 1;
	}; 

	//virtual int  wm_keydown(int key); 
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif