#ifndef LABEL_HEADER
#define LABEL_HEADER

#include "gui_window.h"
#include "../Graphics/CRenderer.h"

extern CRenderer* renderer;

class gui_label: public gui_window
{
	bool center;
	bool alignright;
public:
	gui_label() {text[0] = 0; alignright = true; center = false; this->multiline = false;this->m_inactivecolor = 0xFFFFFFFF;};
	~gui_label() {};

	char text[256];

	void setcolor(COLOR color)
	{
		this->m_inactivecolor = color;
	};

	void settext(char* text)
	{
		strcpy(this->text, text);
	};

	void Center(bool b)
	{
		this->center = b;
	}

	void AlignRight(bool b)
	{
		this->alignright = b;
	}

	bool multiline;
	void SetMultiline(bool tf)
	{
		this->multiline = tf;
	}

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		Rect r = this->m_position;
		r.top += y;
		r.bottom += y;
		r.left += x;
		r.right += x;
		if (this->multiline || this->alignright)
		{
			renderer->DrawText(r, this->text, this->m_inactivecolor, alignright ? ALIGN_RIGHT : 0);
		}
		else
		{
			if (center == false)
				renderer->DrawText(r.left + 10, r.top, this->text, this->m_inactivecolor);
			else
				renderer->DrawCenteredText(r,this->text, this->m_inactivecolor);
		}
		return 1;
	}; 

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 
	virtual int  wm_lclick(coord x, coord y) 
	{
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