#include "gui_window.h"

#include "../CInput.h"

gui_window::gui_window()
{
	this->m_bIsShown = true;
	this->m_position.bottom = 0;
	this->m_position.left = 0;
	this->m_position.right = 0;
	this->m_position.top = 0;
	this->m_caption = 0;
	this->m_parent = 0;
}

gui_window::~gui_window()
{
	if (this->m_caption)
		delete this->m_caption;
}

void gui_window::init()
{
}

void gui_window::DeleteChildren()
{
	while (this->m_subwins.size() > 0)
	{
		gui_window* w = 0;
		w = this->m_subwins.front();
		this->m_subwins.pop_front();

		if (w)
			delete w;
	}
};

int gui_window::renderall(coord x, coord y, coord mx, coord my, int drawme)
{
	if (this->m_bIsShown == false)
		return 0;

	Rect r = this->m_position;
	if (drawme)
	{
		if (my < r.bottom && my > r.top && mx > r.left && mx < r.right)
			this->wm_paint(x, y, mx, my, true);
		else
			this->wm_paint(x, y, mx, my, false);
	}

	//this gets the mouse position relative to my 0,0
	this->screentoclient(mx, my);

	//get position to render clients relative to
	int x2 = this->m_position.left + x;
	int y2 = this->m_position.top + y;

	int i = 1;
	for (auto it = m_subwins.begin(); it != m_subwins.end(); it++)
	{
		gui_window* win = 0;
		win = *it;

		//debug bounds
		/*if (win)
		{
		RECT n = win->m_position;
		n.top += y2;
		n.bottom += y2;
		n.left += x2;
		n.right += x2;
		renderer->SetTexture(0,0);
		renderer->DrawRect(&n, 0xFFFFFFFF);
		}*/

		if (win)
		{
			if (i == this->current_selection)
			{
				win->renderall(x2, y2, win->m_position.left+1, win->m_position.top+1, true);
			}
			else
				win->renderall(x2, y2, mx, my, true);
		}

		i++;
	}
	return 1;
}

gui_window* gui_window::findchildatcoord(coord x, coord y, int flags)//picks topmost
{
	gui_window* topmost = 0;
	for (auto it = m_subwins.begin(); it != m_subwins.end(); it++)
	{
		gui_window* win = 0;
		win = *it;
		if (win == 0 || win->m_bIsShown == false)
			continue;

		Rect r = win->m_position;

		if (y < r.bottom && y > r.top && x > r.left && x < r.right)
		{
			topmost = win;
		}
	}
	return topmost;
}

#include "gui_button.h"
void gui_window::Enter()
{
	// get the selected item and hit enter on it
	if (this->current_selection <= 0)
		return;

	auto iter = this->m_subwins.begin();
	for (int i = 1; i < current_selection; i++)
	{
		iter++;
	}

	gui_window* win = *iter;
	gui_button* button = dynamic_cast<gui_button*>(win);
	if (button)
	{
		button->wm_lclick(0, 0);
	}

	current_selection = 0;// clear it so we go back to hidden
}

int gui_window::wm_paint(coord x, coord y, coord mx, coord my, bool mo)
{
	return 0;
}

int gui_window::wm_keydown(int key)
{
	if (this->m_subwins.empty())
		return 0;

	gui_window* w = this->m_subwins.back();
	if (w && w->m_bIsShown)
	{
		w->wm_keydown(key);
	}
	return 1;
}

int gui_window::wm_char(int key)
{
	if (this->m_subwins.empty())
		return 0;

	gui_window* w = this->m_subwins.back();
	if (w && w->m_bIsShown)
	{
		w->wm_char(key);
	}
	return 1;
}

void gui_window::setpos(coord x, coord y)
{
	int h = m_position.bottom - m_position.top;
	int w = m_position.right - m_position.left;

	this->m_position.top = y; this->m_position.bottom = y + h;
	this->m_position.left = x; this->m_position.right = x + w;
}

void gui_window::setsize(coord width, coord height)
{
	this->m_position.bottom = this->m_position.top + height;
	this->m_position.right = this->m_position.left + width;
}

int gui_window::getheight()
{
	return this->m_position.bottom - this->m_position.top;
}

int gui_window::getwidth()
{
	return this->m_position.right - this->m_position.left;
}

int gui_window::wm_lclick(coord x, coord y)
{
	this->screentoclient(x, y);

	gui_window* w = this->findchildatcoord(x, y, 0);
	if (w && w->m_bIsShown)
	{
		return w->wm_lclick(x, y);
	}
	return 0;
}

int gui_window::wm_lbuttondown(coord x, coord y)
{
	this->screentoclient(x, y);

	gui_window* w = this->findchildatcoord(x, y, 0);
	if (w && w->m_bIsShown)
	{
		w->wm_lbuttondown(x, y);
	}
	return 0;
}

int gui_window::wm_lbuttonup(coord x, coord y)
{
	this->screentoclient(x, y);

	gui_window* w = this->findchildatcoord(x, y, 0);
	if (w && w->m_bIsShown)
	{
		return w->wm_lbuttonup(x, y);
	}
	return 0;
}

int gui_window::wm_ldrag(coord x, coord y)
{
	this->screentoclient(x, y);

	gui_window* w = this->findchildatcoord(x, y, 0);
	if (w && w->m_bIsShown)
	{
		return w->wm_ldrag(x, y);
	}
	return 0;
}

int gui_window::wm_rclick(coord x, coord y)
{
	this->screentoclient(x, y);

	gui_window* w = this->findchildatcoord(x, y, 0);
	if (w && w->m_bIsShown)
	{
		return w->wm_rclick(x, y);
	}
	return 0;
}

void gui_window::screentoclient(coord &x, coord &y)
{
	x -= this->m_position.left;
	y -= this->m_position.top;
}

void gui_window::clienttoscreen(coord &x, coord &y)
{
	x += this->m_position.left;
	y += this->m_position.top;
}

/****************************************************************************

AddWindow: adds a window to this window's subwin array

****************************************************************************/
int gui_window::AddWindow(gui_window *w)
{
	if (!w) return(-1);
	// only add it if it isn't already in our window list.
	m_subwins.push_back(w);
	w->setparent(this);
	return(0);
}

/****************************************************************************

removewindow: removes a window from this window's subwin array

****************************************************************************/
int gui_window::RemoveWindow(gui_window *w)
{
	w->setparent(NULL);
	m_subwins.remove(w);
	return 1;
}

/****************************************************************************

bringtotop: bring this window to the top of the z-order.  the top of the
z-order is the HIGHEST index in the subwin array.

****************************************************************************/
void gui_window::bringtotop(void)
{
	if (m_parent) {
		// we gotta save the old parent so we know who to add back to
		gui_window *p = m_parent;
		p->RemoveWindow(this);
		p->AddWindow(this);
	}
}

/****************************************************************************

isactive: returns true if this window is the active one (the one with input
focus).

****************************************************************************/
bool gui_window::isactive(void)
{
	if (!m_parent) return(1);
	if (!m_parent->isactive()) return(0);
	return(this == m_parent->m_subwins.back());//[m_parent->m_subwins.size()-1]);
}

bool gui_window::process_mouse_events(int eventId, int x, int y)
{
	if (eventId == ENG_L_DOWN)
	{
		if (this->wm_lbuttondown(x, y))
			return true;//dont bother passing it down if we capture it
	}
	else if (eventId == ENG_R_UP)
	{
		if (this->wm_rclick(x, y))
			return true;//dont bother passing it down if we capture it
	}
	else if (eventId == ENG_L_UP)
	{
		if (this->wm_lclick(x, y))
			return true;//dont bother passing it down, we captured it
	}
	else if (eventId == ENG_L_DRAG)
	{
		if (this->wm_ldrag(x, y))
			return true;//dont bother passing it down if we capture it
	}
	return false;
}