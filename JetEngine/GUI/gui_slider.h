#ifndef _SLIDER_HEADER
#define _SLIDER_HEADER

#include <functional>
#include "gui_window.h"
//finish me
class gui_slider: public gui_window
{
public:
	gui_slider()
	{
		drag = false;
		this->position = 0;
		this->max = 100;
		this->setsize(150,40);
	}

	~gui_slider()
	{
	}

	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	}

	double position;
	double max;
	double min;
	void SetRange(double min, double max)
	{
		this->max = max;
		this->min = min;
	}

	//void (*callback)(int pos);
	std::function<void(int)> callback;

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover);

	//virtual int  wm_rendermouse(coord x, coord y);
	bool drag;
	int drag_offset;
	virtual int wm_lbuttondown(coord x, coord y)
	{
		//record vertical offset
		drag = true;
		this->screentoclient(x,y);

		double frac = this->position/1000.0;
		double range = abs(this->m_position.right - this->m_position.left) - 40;//range to place

		int pos = range*frac;
		drag_offset = pos-x;//-y
		return 1;
	}

	virtual int  wm_lbuttonup(coord x, coord y)
	{
		drag = false;
		return 1;
	}

	double GetValue()
	{
		return (this->position / 1000.0)*(max - min) + min;
	}

	virtual int wm_ldrag(coord x, coord y)
	{
		if (drag)
		{
			this->screentoclient(x,y);
			//this->position = y - drag_offset;
			float frac = (float)this->position/(float)max;
			int range = abs(this->m_position.right - this->m_position.left) - 40;//range to place

			this->position = (((double)(x + drag_offset))/((double)range))*1000.0;

			if (position < 0)
				position = 0;
			else if (position > 1000.0)
				position = 1000.0;

			if (callback)
				this->callback((int)position);
		}
		return 0;
	}

	virtual int wm_lclick(coord x, coord y)
	{
		if (drag == false)
		{
			this->screentoclient(x,y);
			if (x > this->position)
				this->position++;
			else
				this->position--;

			if (position < 0)
				position = 0;
			else if (position > 1000)
				position = 1000;

			if (this->callback)
				callback((int)position);
		}

		return 1;
	}
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); };
	//virtual int  wm_cansize(coord x, coord y);
	//virtual int  wm_size(coord x, coord y, int cansize);
	//virtual int  wm_sizechanged(void) { return(0); }
	//virtual int  wm_update(int msdelta) { return(0); }
};

#endif