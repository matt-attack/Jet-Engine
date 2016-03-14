#ifndef _RECT_CLASS_HEADER
#define _RECT_CLASS_HEADER

struct Point
{
	int x,y;

	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};

struct Rectangle
{
	int left, right, top, bottom;

	Rectangle(int l, int r, int t, int b)
	{
		this->left = l;
		this->right = r;
		this->top = t;
		this->bottom = b;
	}

	void operator += (Point p)
	{
		this->left += p.x;
		this->right += p.x;
		this->top += p.y;
		this->bottom += p.y;
	}
};
#endif