#ifndef lolzolsd
#define lolzolsd

#include "gui_window.h"
#include "../Graphics/CRenderer.h"
#include "../Graphics/font.h"

extern CRenderer* renderer;
extern void SetShowKB(bool s);

class gui_textbox: public gui_window
{
	int cursor;
public:
	gui_textbox() {
		this->text[0] = 0;
	};
	~gui_textbox() {};

	char text[128];

	void init()
	{
		this->cursor = 0;
		this->m_inactivecolor = 0xFFFFFFFF;
	};

	void settext(char* text)
	{
		strcpy(this->text, text);
	};

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
	{
		Rect r = this->m_position;
		r.bottom += y;
		r.top += y;
		r.left += x;
		r.right += x;
		renderer->SetPixelTexture(0, renderer->gui_texture);
		//renderer->DrawRect(&this->m_position, 0xffa0a0a0);
		renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, 0xffa0a0a0);
		Rect n = r;//this->m_position;
		n.top+=2; n.bottom-=2; n.left+=2; n.right-=2;
		//renderer->DrawRect(&n, 0xFFFFFFFF);
		renderer->DrawRectUV(&n, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, 0xFFFFFFFF);
		n.left += 5;
		//renderer->DrawText(this->m_position.left + 10, this->m_position.top, this->text, 0xFFFFFFFF);
		renderer->DrawVerticalCenteredText(n, this->text, 0xFFFFFFFF);

		if (this->isactive())//draw cursor if active
		{
			int i = renderer->font->TextSize(this->text,this->cursor);
			if (this->cursor == 0)
				i = 0;
			n.left += i - 5;
			renderer->DrawVerticalCenteredText(n, "|", 0xFFFFFFFF);
		}
		//drawRect(xPos - 1, yPos - 1, xPos + width + 1, yPos + height + 1, 0xffa0a0a0);
		//drawRect(xPos, yPos, xPos + width, yPos + height, 0xff000000);

		return 1;
	}; 

	//virtual int  wm_rendermouse(coord x, coord y);
	//virtual int  wm_lbuttondown(coord x, coord y);
	//virtual int  wm_lbuttonup(coord x, coord y); 
	//virtual int  wm_ldrag(coord x, coord y); 
	virtual int  wm_lclick(coord x, coord y) 
	{
#ifdef ANDROID
		SetShowKB(true);
#endif
		this->screentoclient(x,y);
		if (true)//this->isactive())
		{
			this->cursor = strlen(this->text);
			for (unsigned int i = 0; i < strlen(this->text); i++)
			{
				int l = renderer->font->TextSize(this->text,i+1);
				if (x - 5 < l)
				{
					this->cursor = i;
					break;
				}
			}
		}
		else
			this->cursor = strlen(this->text);
		this->bringtotop();//start accepting input
		return 1;
	};

	virtual int wm_keydown(int key)
	{
		int len = strlen(this->text);
		if (key == VK_LEFT && cursor > 0)
			this->cursor -= 1;
		else if (key == VK_RIGHT && cursor < len)
			this->cursor += 1;
#ifndef ANDROID
		else if (key == VK_DELETE)
		{
			int l = strlen(this->text);
			if (l == 0 || this->cursor == l)
				return 1;

			if (this->cursor == l-1)
				this->text[l-1] = 0;
			else
			{
				memmove(&this->text[cursor], &this->text[cursor+1],l-cursor);
				this->text[l-1] = 0;
			}
		}
#endif

		return 0;
	}

	virtual int wm_char(int key) 
	{
		unsigned int Char = key;//MapVirtualKey(key, MAPVK_VK_TO_CHAR);
#ifndef ANDROID
		if (key == 0 || key == VK_BACK)
#else
		if (key == 0)
#endif
		{
			int l = strlen(this->text);
			if (l == 0 || this->cursor == 0)
				return 1;

			if (this->cursor == l)
				this->text[l-1] = 0;
			else
			{
				memmove(&this->text[cursor-1], &this->text[cursor],l-cursor);
				this->text[l-1] = 0;
			}

			this->cursor -= 1;
		}
		else
		{
			if (Char != '\t' && Char != '\r')
			{
				if (this->cursor == strlen(this->text))
					strcat(this->text, (char*)&Char);
				else
				{
					int l = strlen(this->text)+1;

					memmove(this->text+this->cursor+1,this->text+this->cursor,l-this->cursor);
					this->text[this->cursor] = Char;
				}
				this->cursor++;
			}
		}
		return 1;
	}; 
	//virtual int  wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int  wm_cansize(coord x, coord y); 
	//virtual int  wm_size(coord x, coord y, int cansize); 
	//virtual int  wm_sizechanged(void) { return(0); } 
	//virtual int  wm_update(int msdelta) { return(0); }
};
#endif
