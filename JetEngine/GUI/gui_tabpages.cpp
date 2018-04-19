#include "gui_tabpages.h"

int gui_tabbox::wm_paint(coord x, coord y, coord mx, coord my, bool mouseover)
{
	Rect brect = this->m_position;
	brect.top += y;
	brect.bottom += y;
	brect.left += x;
	brect.right += x;

	Rect brect2 = brect;
	brect2.top += 30;
	renderer->SetPixelTexture(0, (Texture)0);
	renderer->DrawRect(&brect2, COLOR_ARGB(255, 108, 108, 108));

	int tab_widths = 0;
	for (int i = 0; i < this->tabs.size(); i++)
	{
		tab_widths += renderer->TextSize(this->tabs[i].second.c_str()) + 10;
	}
	Rect re = brect;
	re.left += 8;
	re.right = re.left + tab_widths + 2;
	re.bottom = re.top + 30;
	renderer->DrawRect(&re, COLOR_ARGB(255, 85, 85, 85));

	Rect r = brect;
	r.left += 10;
	r.bottom = r.top + 25;
	for (auto ii : this->tabs)
	{
		int width = renderer->TextSize(ii.second.c_str()) + 10;
		Rect r2 = r;
		r2.top += 2;
		r2.right = r2.left + width - 2;
		r2.bottom += 5;
		if (this->current == ii.first)
		{
			renderer->DrawRect(&r2, COLOR_ARGB(255, 108, 108, 108));
			r2.bottom -= 5;
			renderer->DrawCenteredText(r2, ii.second.c_str(), COLOR_ARGB(255, 255, 255, 255));
		}
		else
		{
			renderer->DrawRect(&r2, COLOR_ARGB(255, 64, 64, 64));
			r2.bottom -= 5;
			renderer->DrawCenteredText(r2, ii.second.c_str(), this->m_inactivecolor);
		}

		r.right += width;
		r.left += width;

		if (ii.first == this->current)
			ii.first->Show();
		else
			ii.first->Hide();

		//um wtf this needs to be moved
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

	return 1;
}