#include "gui_slider.h"

int gui_slider::wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
{
	int width = 20;

	Rect r = this->m_position;
	r.top += y;
	r.bottom += y;
	r.left += x;
	r.right += x;

	renderer->SetPixelTexture(0, 0);//renderer->gui_texture);

	unsigned long color = 0xFFFFFFFF;

	renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, COLOR_ARGB(255,64,64,64));
	Rect r2 = r;
	r.bottom -= 2;
	r.top += 2;
	r.left += 2;
	r.right -= 2;
	renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, color);

	char txt[50];
	sprintf(txt, "%0.1f", (float)this->GetValue());
	Rect rr = r;
	rr.left = r.right + 10;
	rr.right = r.right + 500;
	renderer->DrawText(rr, txt, COLOR_ARGB(255,255,255,255), 2);

	if (mouseover)
		color = 0xFFFF00FF;
	else
		color = COLOR_ARGB(255,108,108,108);

	double frac = this->position/1000.0;
	double range = abs(this->m_position.right - this->m_position.left) - 44;//range to place

	int pos = range*frac;
	//fix this last part
	r.left = r.left + pos;
	r.right = r.left + 40;
	//r.bottom = r.top + pos + 80;
	//r.top = r.top + pos;
	renderer->DrawRectUV(&r, 0.0f, 1.0f, 0.0f, 50.0f/512.0f, color);

	return 1;
}