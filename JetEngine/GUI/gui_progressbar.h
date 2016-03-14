
#ifndef GUIPROGRESSBAR_HEADER
#define GUIPROGRESSBAR_HEADER

#include "gui_window.h"
#include "../Graphics/CRenderer.h"

extern CRenderer* renderer;

class gui_progressbar: public gui_window
{
public:
	gui_progressbar() {};
	~gui_progressbar() {};

	float progress;
	char* text;

	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	};

	void settext(char* text)
	{
		this->text = text;
	};

	void setProgress(float frac)
	{
		this->progress = frac;
	};

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		renderer->SetPixelTexture(0, 0);
		renderer->DrawRect(&this->m_position, 0xFF777777);
		Rect r = this->m_position;
		r.bottom -= 3;
		r.left += 3;
		r.right -= 3;
		r.top += 3;
		r.right = r.left + ((float)(r.right - r.left)*this->progress);
		renderer->DrawRect(&r, 0xFF77FF00);
		renderer->DrawText(this->m_position.left + 20, this->m_position.top + 10, this->text, 0xFFFFFFFF);
		return 1;
	}; 

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 
	//virtual int  wm_lclick(coord x, coord y) 
	//{
		//return 1;
	//}; 
	//virtual int  wm_keydown(int key) 
	//{
		//return 1;
	//}; 
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif