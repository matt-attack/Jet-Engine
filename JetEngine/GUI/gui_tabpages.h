#ifndef TABBOX_HEADER
#define TABBOX_HEADER

#include "gui_window.h"
#include "../Graphics/CRenderer.h"
#include "gui_button.h"

extern CRenderer* renderer;

class gui_tabbox: public gui_window
{
	std::vector<std::pair<gui_window*,std::string> > tabs;

	gui_window* current;
public:

	gui_tabbox() 
	{
		this->current = 0;
		this->m_inactivecolor = 0xFFFFFFFF;
		this->Show();
	}

	~gui_tabbox() 
	{
		for (auto ii: this->tabs)
		{
			delete[] ii.first;
		}
	}

	void setcolor(COLOR color)
	{
		this->m_inactivecolor = color;
	}

	gui_window* AddTab(const char* name)
	{
		gui_window* panel = new gui_window;
		panel->setpos(0,30);
		panel->setcaption(name);
		panel->setsize(this->m_position.right - this->m_position.left, 400);
		if (this->tabs.size() > 0)
			this->tabs.back().first->Hide();
		panel->Show();
		this->tabs.push_back(std::pair<gui_window*,std::string>(panel, name));
		this->current = panel;
		this->AddWindow(panel);
		return panel;
	}

	const char* GetCurrentTab()
	{
		return this->current->m_caption;
	}

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		Rect brect = this->m_position;
		brect.top += y;
		brect.bottom += y;
		brect.left += x;
		brect.right += x;

		Rect brect2 = brect;
		brect2.top += 30;
		renderer->SetPixelTexture(0, (Texture)0);
		renderer->DrawRect(&brect2,COLOR_ARGB(255,108,108,108));

		Rect re = brect;
		re.left += 8;
		re.right = re.left + this->tabs.size()*100+2;
		re.bottom = re.top + 30;
		renderer->DrawRect(&re, COLOR_ARGB(255,85,85,85));

		Rect r = brect;
		r.left += 10;
		r.right = r.left + 100;
		r.bottom = r.top + 25;
		for (auto ii: this->tabs)
		{
			Rect r2 = r;
			r2.top += 2;
			r2.right -= 2;
			r2.bottom += 5;
			if (this->current == ii.first)
			{
				renderer->DrawRect(&r2, COLOR_ARGB(255,108,108,108));
				r2.bottom -= 5;
				renderer->DrawCenteredText(r2, ii.second.c_str(), COLOR_ARGB(255,255,255,255));
			}
			else
			{
				renderer->DrawRect(&r2, COLOR_ARGB(255,64,64,64));
				r2.bottom -= 5;
				renderer->DrawCenteredText(r2, ii.second.c_str(), this->m_inactivecolor);
			}

			r.right += 100;
			r.left += 100;

			if (ii.first == this->current)
				ii.first->Show();
			else
				ii.first->Hide();

			if (strcmp(this->GetCurrentTab(), "Weapons") == 0)
			{
				int yo = 180 + this->m_position.top;
				int xo = this->m_position.left;
				renderer->DrawCenteredText(Rect(yo, yo, xo + 175, xo + 275), "Center", COLOR_ARGB(255, 255, 255, 255));
				renderer->DrawCenteredText(Rect(yo, yo, xo + 0, xo + 100), "Left Arm", COLOR_ARGB(255, 255, 255, 255));
				renderer->DrawCenteredText(Rect(yo, yo, xo + 350, xo + 450), "Right Arm", COLOR_ARGB(255, 255, 255, 255));
				yo = 140 + this->m_position.top;
				renderer->DrawCenteredText(Rect(yo, yo, xo + 125, xo + 175), "Left", COLOR_ARGB(255, 255, 255, 255));
				renderer->DrawCenteredText(Rect(yo, yo, xo + 250, xo + 350), "Right", COLOR_ARGB(255, 255, 255, 255));
			}
		}

		//Rect r3 = this->m_position;
		//r2.top = r.top + 80;
		//r2.bottom = r.top + 40;
		//renderer->DrawCenteredText(r2,this->text, this->m_inactivecolor);
		return 1;
	}

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 
	virtual int wm_lclick(coord x, coord y) 
	{
		this->screentoclient(x,y);

		gui_window* w = this->findchildatcoord(x,y,0);
		if (y < 30)
		{
			Rect r = this->m_position;
			r.right = 100;
			r.bottom = 30;
			r.top = r.left = 0;
			for (auto ii: this->tabs)
			{
				if (y < r.bottom && y > r.top && x > r.left && x < r.right)
				{
					this->current = ii.first;
					break;
				}
				r.right += 100;
				r.left += 100;
			}
		}
		else if (w)
		{
			this->bringtotop();
			w->wm_lclick(x,y);
		}

		return 1;
	}

	/*ok, lets make bases more configurable, and add other utilities, on other 

	sides of the planets
	also add hangar at end of runway
	make modular*/

	//fix buttons under settigns window being interacted with in GameMenu
	//add wm_char hook to cgame
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif