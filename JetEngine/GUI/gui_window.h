
#ifndef GUI_WINDOW_HEADER
#define GUI_WINDOW_HEADER

#include "../Graphics/CRenderer.h"

#include <list>

typedef int coord;
typedef unsigned long gui_wincolor;

class gui_window
{
	bool m_bIsShown;
	int current_selection = 0;// used for menu item selection with a controller
public:
	gui_window();
	virtual ~gui_window();
	virtual void init(void);

	gui_window *getparent(void) { return(m_parent); }
	void setparent(gui_window* p) {this->m_parent = p;};

	/////////////
	// window management controls
	/////////////

	int AddWindow(gui_window *w);
	int RemoveWindow(gui_window *w);
	void DeleteChildren();

	// Handling menu item selection
	void Up()
	{
		if (current_selection > 1)
			current_selection--;
	}
	void Down()
	{
		if (current_selection < this->m_subwins.size())// todo need to handle only the right type of labels
			current_selection++;
	}
	void Enter();

	// Hiding and showing
	void Show(void) { m_bIsShown = true; }
	void Hide(void) { m_bIsShown = false; }
	bool IsShown(void) { return(m_bIsShown); }
	void bringtotop(void);
	bool isactive(void);

	void setcaption(const char* caption)
	{
		unsigned int l = strlen(caption);
		this->m_caption = new char[l+1];
		strcpy(this->m_caption, caption);
	}

	/////////////
	// coordinates
	/////////////  

	void setpos(coord x1, coord y1); // boring
	void setsize(coord width, coord height); // boring
	int getheight();
	int getwidth();

	void screentoclient(coord &x, coord &y);
	void clienttoscreen(coord &x, coord &y);

	virtual gui_window *findchildatcoord(coord x, coord y, int flags = 0);

	bool process_mouse_events(int eventId, int x, int y);

	///////////////
	// Drawing Code
	///////////////

	// renders this window + all children recursively
	int renderall(coord x, coord y, coord mx, coord my, int drawme = 1); 


	//////////////
	// Window Messages for input
	//////////////

	int calcall(void); 

	virtual int wm_paint(coord x, coord y, coord mx, coord my, bool mouseover);
	//virtual int wm_rendermouse(coord x, coord y);
	virtual int wm_rclick(coord x, coord y);
	virtual int wm_lbuttondown(coord x, coord y);
	virtual int wm_lbuttonup(coord x, coord y); 
	virtual int wm_ldrag(coord x, coord y); 
	virtual int wm_lclick(coord x, coord y); 
	virtual int wm_keydown(int key);
	virtual int wm_char(int key);
	//virtual int wm_command(gui_window *win, int cmd, int param) { return(0); }; 
	//virtual int wm_cansize(coord x, coord y); 
	//virtual int wm_size(coord x, coord y, int cansize); 
	//virtual int wm_sizechanged(void) { return(0); } 
	//virtual int wm_update(int msdelta) { return(0); }

	//protected:

	//virtual void copy(gui_window &r); // deep copies one window to another

	gui_window *m_parent;
	std::list<gui_window*> m_subwins;
	Rect m_position;

	// active and inactive colorsets
	gui_wincolor m_activecolor;
	gui_wincolor m_inactivecolor;

	// window caption
	char* m_caption;
};

#endif