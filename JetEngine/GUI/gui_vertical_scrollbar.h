#ifndef _VERT_SCROLLBAR_HEADER
#define _VERT_SCROLLBAR_HEADER

#include "gui_window.h"
//finish me
class gui_vertical_scrollbar: public gui_window
{
public:
	gui_vertical_scrollbar()
	{
		drag = false;
		this->position = 0;
		this->max = 100;
	};

	~gui_vertical_scrollbar()
	{
	};

	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	};

	int max;
	void SetRange(int range)
	{
		max = range;
		if (this->position > max)
			position = max;
	};

	int position;
	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		int width = 20;

		Rect r = this->m_position;
		r.top += y;
		r.bottom += y;
		r.left += x;
		r.right += x;

		renderer->SetPixelTexture(0, (Texture)0);//renderer->gui_texture);

		unsigned long color = 0xFFFFFFFF;

		renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, color);
		if (mouseover)
			color = 0xFFFF00FF;
		else
			color = COLOR_ARGB(255,128,128,128);

		float frac = 0;
		if (this->position > 0)
			frac = (float)this->position/(float)max;
		int range = abs(this->m_position.top - this->m_position.bottom) - 80;//range to place

		int pos = (float)range*frac;
		r.bottom = r.top + pos + 80;
		r.left += 2;
		r.right -= 2;
		r.top = r.top + pos;
		renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, color);

		//char txt[50];
		//sprintf(txt, "%d", this->position);
		//renderer->DrawText(r.left - 50, r.bottom, txt, COLOR_ARGB(255,255,255,255));
		
		return 1;
	};

	//virtual int  wm_rendermouse(coord x, coord y);
	bool drag;
	int drag_offset;
	virtual int wm_lbuttondown(coord x, coord y)
	{
		//record vertical offset
		drag = true;
		this->screentoclient(x,y);

		float frac = 0;
		if (max > 0)
			frac = (float)this->position/(float)max;
		int range = abs(this->m_position.top - this->m_position.bottom) - 80;//range to place

		int pos = (float)range*frac;
		drag_offset = pos-y;
		return 1;
	};

	virtual int  wm_lbuttonup(coord x, coord y)
	{
		drag = false;
		return 1;
	};

	virtual int wm_ldrag(coord x, coord y)
	{
		if (drag)
		{
			this->screentoclient(x,y);
			//this->position = y - drag_offset;
			float frac = (float)this->position/(float)max;
			int range = abs(this->m_position.top - this->m_position.bottom) - 80;//range to place

			this->position = (((float)(y + drag_offset))/((float)range))*(float)max;

			if (position < 0)
				position = 0;
			else if (position > this->max)
				position = max;
		}
		return 0;
	};

	virtual int wm_lclick(coord x, coord y)
	{
		if (drag == false)
		{
			log("clicked bar\n");
			this->screentoclient(x,y);
			if (y > this->position)
				this->position++;
			else
				this->position--;

			if (position < 0)
				position = 0;
			else if (position > max)
				position = max;
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