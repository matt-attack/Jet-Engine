
#ifndef GUIDROPBOX_HEADER
#define GUIDROPBOX_HEADER

#include "gui_window.h"
#include "../Sound/SoundObject.h"
#include "../Graphics/CRenderer.h"

extern CRenderer* renderer;
extern void log3(char* var);

class gui_dropbox: public gui_window
{
	bool open;
	int selected;
	std::vector<std::string> items;
public:
	gui_dropbox()
	{
		this->selected = 0;
		this->open = false;
		this->text = 0;
		this->setsize(100,40);
		this->callback = 0;
	};

	~gui_dropbox()
	{
		if (this->text)
			delete[] this->text;
	};

	char* text;

	void init()
	{
		this->m_inactivecolor = 0xFFFFFFFF;
	};

	void settext(char* txt)
	{
		if (this->text)
			delete[] this->text;

		unsigned int l = strlen(txt);
		this->text = new char[l+1];
		strcpy(this->text, txt);
	};

	void AddItem(const std::string& item)
	{
		this->items.push_back(item);
	}

	void SetItem(const std::string& item)
	{
		for (int i = 0; i < this->items.size(); i++)
		{
			if (item == items[i])
			{
				selected = i;
				break;
			}
		}
	}

	void ClearItems()
	{
		this->items.clear();
	}

	void (*callback)(gui_dropbox*);

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		Rect r = this->m_position;
		r.top += y;
		r.bottom += y;
		r.left += x;
		r.right += x;

		if (this->open)
			r.bottom -= 25*this->items.size();

		renderer->SetPixelTexture(0,renderer->gui_texture);

		unsigned long color = 0xFFFFFFFF;
		if (mouseover)
			color = 0xFFFF00FF;
		
		renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, color);
		if (this->items.size() == 0)
			return 1;

		Rect tr = r;
		tr.left -= 115;
		if (this->text)
			renderer->DrawVerticalCenteredText(tr, this->text, 0xFFFFFFFF);

		renderer->DrawCenteredText(r, this->items[selected].c_str(), 0xFFFFFFFF);

		if (this->open)
		{
			Rect r = this->m_position;
			r.top += y + 40;
			r.bottom = r.top + 25*this->items.size();
			r.left += x;
			r.right += x;
			renderer->DrawRectUV(&r, 0,1,0,50.0f/512.0f, color);
			r.bottom = r.top + 25;
			for (int i = 0; i < this->items.size(); i++)
			{
				renderer->DrawCenteredText(r, this->items[i].c_str(), 0xFFFFFFFF);
				r.bottom += 25;
				r.top += 25;
			}
		}
		//renderer->DrawText(this->m_position.left + 10, this->m_position.top, this->text, 0xFFFFFFFF);
		return 1;
	};

	std::string GetSelected()
	{
		return this->items[this->selected];
	}

	void SetSelected(int id)
	{
		this->selected = id;
	}

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y);
	//virtual int  wm_ldrag(coord x, coord y);
	virtual int  wm_lclick(coord x, coord y)
	{
		//log("clicked button\n");
		//SoundManager::GetInstance()->GetSound("select")->Play();
		//SoundManager::GetInstance()->GetSound("select")->SetRelative(true);
		this->screentoclient(x,y);
		//determine item clicked on
		if (open)
		{
			int index = (y-40)/25;
			if (y > 40 && index < this->items.size())
			{
				this->selected = index;
				if (this->callback)
					callback(this);
			}
		}
		else
		{
			this->bringtotop();
		}

		this->open = !this->open;
		if (open)
			this->m_position.bottom += 25*this->items.size();
		else
			this->m_position.bottom -= 25*this->items.size();

		return 1;
	};

	virtual int  wm_keydown(int key)
	{
		//use arrow keys
		
		return 1;
	};
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); };
	//virtual int  wm_cansize(coord x, coord y);
	//virtual int  wm_size(coord x, coord y, int cansize);
	//virtual int  wm_sizechanged(void) { return(0); }
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif